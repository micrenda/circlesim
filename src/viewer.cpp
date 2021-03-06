#include <irrlicht/irrlicht.h>
#include <irrlicht/driverChoice.h>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <getopt.h>
#include <math.h>

#include "type.hpp"
#include "config.hpp"
#include "csv.h"
#include "util.hpp"
#include "gradient.hpp"
#include "time_controller.hpp"
#include "frame_controller_interaction.hpp"
#include "frame_controller_field.hpp"


using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


#define VECT_X  1
#define VECT_Y  1
#define VECT_Z -1

void print_help()
{
    std::string exe_name = "viewer";
    
    printf("Usage:\n\n");
    printf("  %s <base_directory> -i <n> [-f <name>.<n>]\n", exe_name.c_str());
    printf("  %s -h\n", exe_name.c_str());
    printf("\n");
    printf("  -h  --help               Print this help menu\n");
    printf("  -i  --interaction        Specify which interaction to render\n");
    printf("  -f  --field <name>.<r>   Specify a field to be rendered\n");
    printf("                           (It is possible to specify different fields but \n");
    printf("                            only if they refer to different planes        )\n");
    printf("\n");
}

void print_key_summary()
{
    printf("┌────────────────────────────────────────────────────────────┐\n");
    printf("│ Keybindings                                                │\n");
    printf("├────────────────────────────────────────────────────────────┤\n");
    printf("│ ← → ↑ ↓            Move camera left, right, up, down       │\n");
    printf("│ + -                Increase / decrease movie speed by 10%%  │\n");
    printf("│ * /                Increase / decrease movie speed by 10x  │\n");
    printf("│ Page Dw, Page Up   Zoom in  / zoom out                     │\n");
    printf("│ .                  Play movie forward                      │\n");
    printf("│ ,                  Play movie backward                     │\n");
    printf("│ Space              Stop / Play movie                       │\n");
    printf("│ [ ]                Increase / decrease field opacity       │\n");
    printf("│ P                  Toggle field white background           │\n");
    printf("│ Q                  Quit                                    │\n");
    printf("└────────────────────────────────────────────────────────────┘\n");
}

class MyEventReceiver : public IEventReceiver
{
    
private:
    bool keys_store[KEY_KEY_CODES_COUNT];
    
public:

    MyEventReceiver()
    {
        for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
            keys_store[i] = false;
    }

    virtual bool OnEvent(const SEvent& event)
    {
        if (event.EventType == irr::EET_KEY_INPUT_EVENT)
            keys_store[event.KeyInput.Key] = event.KeyInput.PressedDown;

        return false;
    }

    // This is used to check whether a key is being held down
    virtual bool isKeyDown(EKEY_CODE keyCode) const
    {
        return keys_store[keyCode];
    }

};


int last_fps = -1;
unsigned int then;
const float speed_linear  = 15.f  / (1 / AU_TIME);
const float speed_angular = 15.f/ (1 / AU_TIME) ;

// This value is used to convert between real length (in meters) into pixel length
double length_au_to_pixels_ratio;




void load_field(FieldMovieConfig cfg, fs::path dat_file, FieldMovie& field_movie, unsigned int subrender_id, bool debug = false)
{
    ifstream file(dat_file.string(), ios::in | ios::binary);
    
    field_movie.frames = new FieldMovieFrame[cfg.nt];
    
    unsigned int len = cfg.na * cfg.nb;
    
    
    FieldMovieSubConfig& subrender = cfg.subrenders[subrender_id];
    Gradient gradient = Gradient(subrender.color, subrender.value_min, subrender.value_max, subrender.value_min_abs, subrender.value_max_abs, debug);
    
    double lowest  = gradient.get_lowest_value();
    double highest = gradient.get_highest_value();
    // Initializing palette color
    for (unsigned int i = 0; i < UCHAR_MAX; i++)
    {
        field_movie.palette[i] = gradient.get_color(lowest + (highest - lowest) / (UCHAR_MAX+1) * i);
    }
    
    
    
    
    
    double* values = new double[len];
    
    printf("Selected %s.%u (%s) %u frames of %u x %u pixels (memory usage: %.2f Mb): loading file %s ",  cfg.name.c_str(), subrender_id, subrender.title.c_str(), cfg.nt, cfg.na, cfg.nb, sizeof(unsigned char) * cfg.na * cfg.nb * cfg.nt / 1024.f / 1024.f, dat_file.filename().c_str()); 
    cout<<flush;
    
    for (unsigned int t = 0; t < cfg.nt; t++)
    {
        
        FieldMovieFrame& frame = field_movie.frames[t];
        
        file.read((char*)values, sizeof(double) * len);
        
        frame.time   = (cfg.time_start + (cfg.time_end - cfg.time_start) / cfg.nt * t);
        frame.values = new unsigned char[len];
        
        for (unsigned int i = 0; i < len; i++)
        {
            double value = values[i];
            
            if (value < lowest)
                frame.values[i] = 0;
            else if (value > highest)
                frame.values[i] = UCHAR_MAX;
            else
                frame.values[i] = (value - lowest) / (highest - lowest) * (UCHAR_MAX+1);
        }
        
        printf(".");
        cout<<flush;
    }
    printf(" done.\n");
    cout<<flush;
    
    file.close();
    
    }



void update_camera_position(IGUIEditBox* text_camera,  vector3df camera_position, ILightSceneNode* light_node)
{
    float rho, theta, phi;
    
    cartesian_to_spherical(camera_position.X / VECT_X, camera_position.Y / VECT_Y, camera_position.Z / VECT_Z, theta, phi);
    
    rho = vector_module(camera_position.X / VECT_X, camera_position.Y / VECT_Y, camera_position.Z / VECT_Z);
    
    float light_x, light_y, light_z;
    spherical_to_cartesian(200.f, theta, phi, light_x, light_y, light_z);
    
    light_node->setPosition(vector3df(light_x * VECT_X, light_y * VECT_Y, light_z * VECT_Z));


    text_camera->setText( (bo::wformat(L"Camera\nx=%1$+.3Em, y=%2$+.3Em, z=%3$+.3Em\nrho=%4$+.3Em, theta=%5$+.0f, phi=%6$+.0f\n")
    % (camera_position.X / VECT_X / length_au_to_pixels_ratio * AU_LENGTH)
    % (camera_position.Y / VECT_Y / length_au_to_pixels_ratio * AU_LENGTH)
    % (camera_position.Z / VECT_Z / length_au_to_pixels_ratio * AU_LENGTH)
    % (rho / length_au_to_pixels_ratio * AU_LENGTH)
    % (theta / M_PI * 180.f)
    % (phi   / M_PI * 180.f)).str().c_str());
}

void load_field_texture(
    FieldMovieConfig& cfg,
    FieldMovieFrame&  frame,
    unsigned int*     palette,
    
    unsigned char render_opacity,
    bool render_unblend,
    
    ITexture* texture)
{
    void* bitmap = texture->lock();
    if (bitmap)
    {       
        unsigned int pitch      = texture->getPitch();
        ECOLOR_FORMAT format    = texture->getColorFormat();
        unsigned int bytes      = IImage::getBitsPerPixelFromFormat(format) / 8;
        
        unsigned int alpha_mask = render_opacity << 24;
        
        for (unsigned int a = 0; a < cfg.na; a++)
        {
            for (unsigned int b = 0; b < cfg.nb; b++)
            {
                unsigned char& palette_id = frame.values[a * cfg.nb + b];
                //printf("[%u,%u] palette %u: #%08x\n", a, b, palette_id, palette[palette_id]);
                unsigned int base_color = palette[palette_id];
                
                if (render_unblend)
                    base_color = unblend_color(base_color, 0x00ffffff);
                else
                {
                    base_color = (base_color & 0x00ffffff) | alpha_mask;
                }
                
                SColor(base_color).getData((unsigned int*)((char*)bitmap + (a * pitch) + (b * bytes)), ECF_A8R8G8B8);
            }
        }

        texture->unlock();
    }
}


void load_debug_texture(FieldMovieConfig& cfg, ITexture* texture)
{
  
	unsigned char color_shift_a, color_shift_b;

	switch (cfg.plane)
	{
		case XY:
			color_shift_a = 16;
			color_shift_b =  8;
		break;
		case XZ:
			color_shift_a = 16;
			color_shift_b =  0;
		break;
		case YZ:
			color_shift_a =  8;
			color_shift_b =  0;
		break;
	}

	void* bitmap = texture->lock();
	if (bitmap)
	{       
		unsigned int pitch      = texture->getPitch();
		ECOLOR_FORMAT format    = texture->getColorFormat();
		unsigned int bytes      = IImage::getBitsPerPixelFromFormat(format) / 8;
		
		for (unsigned int a = 0; a < cfg.na; a++)
		{
			for (unsigned int b = 0; b < cfg.nb; b++)
			{
				SColor( (unsigned int) (((a * 0xff /cfg.na) << color_shift_a) + ((b * 0xff / cfg.nb)<< color_shift_b))).getData((unsigned int*)((char*)bitmap + (a * pitch) + (b * bytes)), ECF_A8R8G8B8);
			}
		}

		texture->unlock();
	}
}



int main(int argc, char *argv[])
{
    fs::path base_dir           = fs::path("");
    fs::path interaction_subdir = fs::path("");
    
    bool ask_video_driver = false;
    
    vector<std::string> flag_selected_fields;
    
    int current_interaction = -1;
    int current_node        = -1;
    
    bool debug = false;

    int flag;
    static struct option long_options[] = {
        {"help",        0, 0, 'h'},
        {"interaction", 1, 0, 'i'},
        {"ask-video-driver",    0, 0, 'w'},
        {"field",   1, 0, 'f'},
        {"debug",   0, 0, 'd'},
        {NULL, 0, NULL, 0}
    };
    
    int option_index = 0;
    while ((flag = getopt_long(argc, argv, "hi:f:d", long_options, &option_index)) != -1)
    {
        switch (flag)
        {
        case 'i':
            current_interaction = stoi(optarg);
            break;
        case 'h':
            print_help();
            exit(0);
            break;
        case 'd':
            debug = true;
            break;
        case 'w':
            ask_video_driver = true;
            break;
        case 'f':
			flag_selected_fields.push_back(std::string(optarg));
            break;
        case '?':
            print_help();
            exit(-1);
            break;
        default:
            printf ("?? getopt returned character code 0%o ??\n", flag);
            exit(-1);
            break;
        }
    }
    

    
    if (optind < argc)
    {
     
        if (optind == argc - 1)
            base_dir = fs::path(argv[optind++]);
        else
        {
            printf("Only one base directory can be provided\n");
            exit(-1);
        }
    }
    
    if (base_dir == fs::path(""))
    {
        printf("Please specify the base directory.\n");

        exit(-1);
    }
    
    if (!fs::is_directory(base_dir))
    {
        printf("Directory '%s' does not exists\n", base_dir.c_str());
        exit(-1);
    }
        
    if (current_interaction < 0)
    {
        printf("Please specify the interaction using the \"-i <n>\" flag.\n");
        exit(-1);
    }


    // Getting current node
    static const bo::regex e("^i([0-9]+)n([0-9]+)$");
    
    fs::directory_iterator end_iter;    
    for( fs::directory_iterator dir_iter(base_dir) ; dir_iter != end_iter ; ++dir_iter)
    {
        if (fs::is_directory(dir_iter->status()))
        {
            std::string dir_name = std::string(dir_iter->path().filename().string());
            
            bo::match_results<std::string::const_iterator> what;
            if (bo::regex_match(dir_name, what, e))
            {
                if (stoi(what[1]) == current_interaction)
                {
                    current_node = stoi(what[2]);
                    break;
                }
            }
        }
    }
    
    
    if (current_node < 0)
    {
        printf("Unable to find interaction %d in directory '%s'.\n", current_interaction, base_dir.c_str());
        exit(-1);
    }
    
    
    interaction_subdir = fs::path((bo::format("i%dn%d") % current_interaction % current_node).str());

    // Reading the configuration files
    
    fs::path cfg_file = base_dir / fs::path("parameters_si.cfg");
    
    
    Simulation          simulation;
    Pulse               laser;
    Particle            particle;
    Laboratory                  laboratory;
    ParticleStateGlobal         particle_state;
    vector<FieldRender>         field_renders;
    vector<ResponseAnalysis>    response_analyses;
    vector<std::string> headers;
    vector<std::string> sources;
    
    read_config(cfg_file, simulation, laser, particle, particle_state, laboratory, field_renders, response_analyses, headers, sources);
    
    
    
    
    
    
    // Reading input file to count how many record there are inside
    unsigned int records_count = 0;
    
    ifstream f((base_dir / interaction_subdir / fs::path("interaction.csv")).string());
    std::string line;
    
    while (std::getline(f, line)) records_count++;
    
    if (records_count > 0) // Removing the line containing the header
        records_count--;
    
    if (records_count <= 0)
    {
        printf("In the file '%s' there are no records, unable to trace particle path.\n", "interaction.csv");
        exit(-1);
    }
    
    f.close();
    
    // Read input files
    csv::CSVReader<13, csv::trim_chars<>, csv::no_quote_escape<';'>> in((base_dir / interaction_subdir / fs::path("interaction.csv")).string());
    
    in.read_header(csv::ignore_extra_column,  
        "time",
        "relative_position_x",
        "relative_position_y",
        "relative_position_z",
        "relative_momentum_x",
        "relative_momentum_y",
        "relative_momentum_z",
        "field_e_x",
        "field_e_y",
        "field_e_z",
        "field_b_x",
        "field_b_y",
        "field_b_z");
    
    ParticleRecord* records = (ParticleRecord*) malloc(sizeof(ParticleRecord) * records_count);
    
    unsigned int records_loaded = 0;
    for (unsigned int i = 0; i < records_count; i++)
    {
        ParticleRecord& record = records[i];
        
        double time;
        double relative_position_x;
        double relative_position_y;
        double relative_position_z;
        double relative_momentum_x;
        double relative_momentum_y;
        double relative_momentum_z;
        double field_e_x;
        double field_e_y;
        double field_e_z;
        double field_b_x;
        double field_b_y;
        double field_b_z;

        bool loaded = in.read_row(time,
            relative_position_x,   relative_position_y,   relative_position_z,
            relative_momentum_x, relative_momentum_y, relative_momentum_z,
            field_e_x, field_e_y, field_e_z,
            field_b_x, field_b_y, field_b_z);
            
        if (loaded)
            records_loaded++;
        else
            continue;
            
        record.time                 = time / AU_TIME;
        
        record.relative_position_x  = relative_position_x / AU_LENGTH;
        record.relative_position_y  = relative_position_y / AU_LENGTH;
        record.relative_position_z  = relative_position_z / AU_LENGTH;

        record.relative_momentum_x  = relative_momentum_x / AU_MOMENTUM;
        record.relative_momentum_y  = relative_momentum_y / AU_MOMENTUM;
        record.relative_momentum_z  = relative_momentum_z / AU_MOMENTUM;
        
        record.field_e_x = field_e_x / AU_ELECTRIC_FIELD;
        record.field_e_y = field_e_y / AU_ELECTRIC_FIELD;
        record.field_e_z = field_e_z / AU_ELECTRIC_FIELD;
        
        record.field_b_x = field_b_x / AU_MAGNETIC_FIELD;
        record.field_b_y = field_b_y / AU_MAGNETIC_FIELD;
        record.field_b_z = field_b_z / AU_MAGNETIC_FIELD;
    }
    
    
    
    if (records_count == records_loaded)
        printf("Found and loaded %u records from file '%s' (memory usage: %.2f Mb)\n", records_loaded, fs::path("interaction.csv").c_str(), sizeof(ParticleRecord) * records_loaded / 1024.d / 1024.d);
    else
        printf("WARN: Found %u record but only loaded %d records from file '%s' (memory usage: %.2f Mb)\n", records_count, records_loaded, fs::path("interaction.csv").c_str(), sizeof(ParticleRecord) * records_loaded / 1024.d / 1024.d);

    
    
    
    // Detecting if there was some field render files that could be used
    static const bo::regex regex_render("^field_render_([[:print:]]+)\\.cfg$");
    
    vector<FieldMovieConfig> existing_render_cfgs;
    

    for( fs::directory_iterator file_iter(base_dir/interaction_subdir) ; file_iter != end_iter ; ++file_iter)
    {
        if (fs::is_regular_file(file_iter->status()))
        {
            std::string file_name = std::string(file_iter->path().filename().string());
            
            bo::match_results<std::string::const_iterator> what;
            if (bo::regex_match(file_name, what, regex_render))
            {
                FieldMovieConfig render_cfg;
                render_cfg.name = what[1];
                
                fs::path file =  *file_iter;
                read_config_render_movie(file, render_cfg);
                existing_render_cfgs.push_back(render_cfg);
            }
        }
    }
    
    vector<FieldMovie> 			field_movies;
    vector<FieldMovieConfig> 	field_cfgs;
    
    for (std::string flag_selected_field: flag_selected_fields)
    {
		FieldMovie 		 field_movie;
		FieldMovieConfig field_cfg;
		
		field_movie.palette = new unsigned int[UCHAR_MAX + 1];
		
        static const bo::regex regex_render_flag("^([[:print:]]+)\\.([\\d]+)$");
        bo::match_results<std::string::const_iterator> what;
        if (bo::regex_match(flag_selected_field, what, regex_render_flag))
        {
            std::string  render        = what[1];
            unsigned int subrender     = stoi(what[2]);
            
            
            bool found = false;
            
            for (FieldMovieConfig existing_render_cfg: existing_render_cfgs)
            {
                if (existing_render_cfg.name == render)
                {
                    field_cfg = existing_render_cfg;
                    found = true;
                    break;
                }
            }
            
            // TODO: Check if another field render was loaded for the same plane
            
            if (found)
            {
                if (subrender >= 0 and subrender <= field_cfg.subrenders.size() - 1)
                {
                    std::string dat_filename = (bo::format("field_render_%s_r%u.dat") % field_cfg.name % subrender).str();
                    load_field(field_cfg, base_dir / interaction_subdir / fs::path(dat_filename), field_movie, subrender, debug);
                   
                    field_cfgs.push_back(field_cfg);
                    field_movies.push_back(field_movie);
                    
                }
                else
                {
                    printf("Unable to find the subrender %u. Please specify a subrender between %u and %u.\n", subrender, 0, (unsigned int) field_cfg.subrenders.size() - 1);
                    exit(-1);
                }
            }
            else
            {
                printf("Unable to find the render '%s'\n", render.c_str());
                exit(-1);
            }
        }
        else
        {
            printf("Unable to understand the field flag: %s\n", flag_selected_field.c_str());
            exit(-1);
        }
    }
    
    
    

    
    if (!existing_render_cfgs.empty())
    {
        
        printf("┌────────────────────────────────────────────────────────────┐\n");
        printf("│ Field renders available  ( use --field <render>.<n> )      │\n");
        printf("├────────────────────────────────────────────────────────────┤\n");
        
        for (FieldMovieConfig existing_render_cfg: existing_render_cfgs)
        {
            
            printf("│ %-49s %2u .. %-2u │\n", existing_render_cfg.name.c_str(), 0, (unsigned int) existing_render_cfg.subrenders.size() - 1);

        }
        
        
        printf("└────────────────────────────────────────────────────────────┘\n");
        
    }
    
    
    
    print_key_summary();
    
    
    video::E_DRIVER_TYPE driverType;
    
    if (ask_video_driver)
    {
        driverType = driverChoiceConsole();
        if (driverType == video::EDT_COUNT)
        {
            printf("No video driver selected.\n");
            return 1;
        }
    }
    else
        driverType = video::EDT_OPENGL;

    MyEventReceiver event_receiver;
    IrrlichtDevice *device = createDevice(driverType, dimension2d<u32>(640, 480), 16, false, false, false, &event_receiver);

    if (!device)
        return 1;
        
    device->setWindowCaption(L"Circlesim viewer");
    
    
    vector3df origin = vector3df(  0 * VECT_X,  0 * VECT_Y,  0 * VECT_Z);
    
    IVideoDriver* driver = device->getVideoDriver();
    ISceneManager* smgr = device->getSceneManager();
    IGUIEnvironment* guienv = device->getGUIEnvironment();
    
    //driver->setTransform(video::ETS_WORLD, ISceneNode::AbsoluteTransformation);
    
    smgr->setAmbientLight(video::SColorf(0.2,0.2,0.2,1));
    
    IGUIEditBox* text_camera = guienv->addEditBox(L"Camera position: N/A",  rect<s32>(10,10,200,40), false);
    text_camera->setMultiLine(true);
    
    
    IAnimatedMesh* axis_x_mesh = smgr->addArrowMesh("x-axis", 0xFFAF6767, 0xFFAF6767,  10, 20, 10.0, 8.0, 0.3, 1.0);
    IAnimatedMesh* axis_y_mesh = smgr->addArrowMesh("y-axis", 0xFF67AF67, 0xFF67AF67,  10, 20, 10.0, 8.0, 0.3, 1.0);
    IAnimatedMesh* axis_z_mesh = smgr->addArrowMesh("z-axis", 0xFF6767AF, 0xFF6767AF,  10, 20, 10.0, 8.0, 0.3, 1.0);
     

    vector3df rotation_to_x = vector3df(  0,  0,-90);
    vector3df rotation_to_y = vector3df(  0,  0,  0);
    vector3df rotation_to_z = vector3df(-90,  0,  0);
   
    
    IMeshSceneNode* axis_x_node =  smgr->addMeshSceneNode(axis_x_mesh, 0, -1, origin, rotation_to_x);
    IMeshSceneNode* axis_y_node =  smgr->addMeshSceneNode(axis_y_mesh, 0, -1, origin, rotation_to_y);
    IMeshSceneNode* axis_z_node =  smgr->addMeshSceneNode(axis_z_mesh, 0, -1, origin, rotation_to_z);
    
    
    
    axis_x_node->setMaterialFlag(video::EMF_LIGHTING, true);
    axis_y_node->setMaterialFlag(video::EMF_LIGHTING, true);
    axis_z_node->setMaterialFlag(video::EMF_LIGHTING, true);
    
    //axis_x_node->getMaterial(0).Shininess = 20.0f;
    //axis_x_node->getMaterial(0).SpecularColor.set(80,80,80,80);
    //axis_x_node->getMaterial(0).AmbientColor.set(10,10,10,10);
    //axis_x_node->getMaterial(0).DiffuseColor.set(20,20,20,20);
    //axis_x_node->getMaterial(0).EmissiveColor.set(0,0,0,0); 

    
    float    border_radius = 40;
    unsigned border_sides  = 100;
    SColor   border_color  = SColor(0xff, 0xff, 0xff, 0xff);
    
    length_au_to_pixels_ratio = border_radius / simulation.laser_influence_radius;
    printf("length_to_pixel_ratio = %E\n", length_au_to_pixels_ratio);
    
    float    border_side = border_radius * 2 * M_PI / border_sides;
    
    vector3df border_start_position_xy = vector3df(border_radius, 0, 0);
    vector3df border_start_position_xz = vector3df(0, 0, border_radius);
    vector3df border_start_position_yz = vector3df(0, border_radius, 0);
    
    for (unsigned int s = 0; s < border_sides; s++)
    {
        IMesh* cyl_mesh = smgr->getGeometryCreator()->createCylinderMesh(0.1, border_side, 20, border_color, false, 0.f);
        
        vector3df border_position_xy= border_start_position_xy;
        vector3df border_position_xz= border_start_position_xz;
        vector3df border_position_yz= border_start_position_yz;
        
        border_position_xy.rotateXYBy(360.f / border_sides * (s-0.5f), origin);
        border_position_xz.rotateXZBy(360.f / border_sides * (s-0.5f), origin);
        border_position_yz.rotateYZBy(360.f / border_sides * (s-0.5f), origin);
        
        smgr->addMeshSceneNode(cyl_mesh, 0, -1, border_position_xy, vector3df(0, 0, 360.f * s / border_sides ));
        smgr->addMeshSceneNode(cyl_mesh, 0, -1, border_position_xz, vector3df(-360.f * s / border_sides, 0  , 90));
        smgr->addMeshSceneNode(cyl_mesh, 0, -1, border_position_yz, vector3df(90+360.f * s / border_sides, 0, 0));
    }
    
    // Particle
    
    //ITexture * particle_texture = driver->addRenderTargetTexture(dimension2d<u32>(128, 128));
    //driver->setRenderTarget(particle_texture);
    //driver->draw2DRectangle(SColor(0xFF, 0x53, 0xA1, 0x62), rect<s32>(position2d<s32>(0,0),position2d<s32>(128,128)));
    
    IAnimatedMesh*  particle_mesh = smgr->addSphereMesh ("particle", 0.30f);
    IMeshSceneNode* particle_node = smgr->addMeshSceneNode(particle_mesh, 0, -1, origin, origin);
    particle_node->setMaterialFlag(video::EMF_LIGHTING, true); 
    //particle_node->getMaterial(0).Shininess = 20.0f;
    
    
    
    vector<ITexture*>           render_plane_textures;
    vector<IMesh*>              render_plane_meshes;
    vector<IMeshSceneNode*>     render_plane_nodes;
    
    
    
    unsigned char render_opacity = 255;
    bool          render_unblend = false;
    
    for (unsigned int i = 0; i < field_cfgs.size(); i++)
    {
		
        ITexture*           render_plane_texture = NULL;
		IMesh*              render_plane_mesh = NULL;
		IMeshSceneNode*     render_plane_node = NULL;
    
		FieldMovieConfig& field_cfg = field_cfgs[i];
		
		std::string plane_id = (bo::format("field_%s") % field_cfg.name).str();
        render_plane_texture = driver->addTexture(dimension2d<u32>(field_cfg.na, field_cfg.nb), plane_id.c_str(), ECF_A8R8G8B8);
        
        double w1, w2;
        vector3df plane_rotation;
        
        
        switch (field_cfg.plane)
        {
            case XY:
                w1 = field_cfg.space_size_x * length_au_to_pixels_ratio;
                w2 = field_cfg.space_size_y * length_au_to_pixels_ratio;
                plane_rotation = vector3df(0.f, 180.f, -90.f);
            break;
            case XZ:
                w1 = field_cfg.space_size_x * length_au_to_pixels_ratio;
                w2 = field_cfg.space_size_z * length_au_to_pixels_ratio;
                plane_rotation = vector3df(-90.f, 90.f, 180.f);
            break;
            case YZ:
                w1 = field_cfg.space_size_y * length_au_to_pixels_ratio;
                w2 = field_cfg.space_size_z * length_au_to_pixels_ratio;
                plane_rotation = vector3df(+90.f, 90.f, 90.f);
            break;
        }
        
        render_plane_mesh = smgr->getGeometryCreator()->createCubeMesh(vector3df(w1, w2, 0.01f));
        
        render_plane_node = smgr->addMeshSceneNode(render_plane_mesh, 0, -1, origin, plane_rotation);
        render_plane_node->setMaterialTexture( 0, render_plane_texture);
        
        IMeshBuffer* render_plane_mesh_buffer = render_plane_mesh->getMeshBuffer(0);
        
        render_plane_mesh_buffer->getTCoords(0).set(0.f, 0.f);
        render_plane_mesh_buffer->getTCoords(1).set(1.f, 0.f);
        render_plane_mesh_buffer->getTCoords(2).set(1.f, 1.f);
        render_plane_mesh_buffer->getTCoords(3).set(0.f, 1.f);
        
        
        render_plane_mesh_buffer->getTCoords(4).set(1.f, 0.f);
        render_plane_mesh_buffer->getTCoords(5).set(1.f, 1.f);
        render_plane_mesh_buffer->getTCoords(6).set(0.f, 1.f);
        render_plane_mesh_buffer->getTCoords(7).set(0.f, 0.f);  
       
    
        
        
        if (debug)
        {
            printf("Created render plane %s with these vertex indexes:\n",field_cfg.name.c_str());
            for (unsigned int i = 0; i < 8; i++)
            {
                vector3df& pos = render_plane_mesh_buffer->getPosition(i);
                vector2df& uv  = render_plane_mesh_buffer->getTCoords(i);
                printf("%u: %f, %f, %f (%f,%f)\n", i, pos.X / VECT_X, pos.Y / VECT_Y, pos.Z / VECT_Z, uv.X, uv.Y );    
                
            }
            
        }
         
                
        render_plane_textures.push_back(render_plane_texture);
		render_plane_meshes.push_back(render_plane_mesh);
		render_plane_nodes.push_back(render_plane_node);
    }
    

    
    
    

    
    // The ration between the time in speed in real (AU) and the speed we see at screen.
    // It is a dimensionless value. A value of 10 means that the value is slowed down 10 times.
    // It is initialized in a way that, at the beginning, the full movie will be played in one minute
    

    bool key_add_previous_status = false;
    bool key_sub_previous_status = false;
    bool key_mul_previous_status = false;
    bool key_div_previous_status = false;
    bool key_period_previous_status = false;
    bool key_comma_previous_status  = false;
    bool key_space_previous_status  = false;
    
    bool key_p_previous_status  = false;
    bool key_q_previous_status  = false;
    
    
    
    
    vector3df camera_position = vector3df(border_radius * VECT_X, border_radius * VECT_Y, border_radius * VECT_Z);
	ICameraSceneNode* camera_node = smgr->addCameraSceneNode(0, camera_position, origin);
    
    
    
    ILightSceneNode* light_node = smgr->addLightSceneNode(0, vector3df(2.* border_radius * VECT_X, 2 * border_radius * VECT_Y, 2.* border_radius * VECT_Z), video::SColorf(1.f, 1.f, 1.f), 5.0 * border_radius);
    
    
    //// This code force Irrlicht to use right hand projection system
    //matrix4 matproj = camera_node->getProjectionMatrix();
	//matproj(0,0) *= -1;
	//camera_node->setProjectionMatrix(matproj);
    
    
       

    then  = device->getTimer()->getTime();
    
    TimeController& time_controller = *new TimeController(records[0].time, records[records_loaded-1].time);
    FrameControllerInteraction& frame_controller_interaction = *new FrameControllerInteraction(records, records_loaded, time_controller);
    vector<FrameControllerField*> frame_controllers_field;
    
    for (unsigned int i = 0; i < field_cfgs.size(); i++)
    {
		frame_controllers_field.push_back(new FrameControllerField(field_movies[i].frames, field_cfgs[i].nt, time_controller));
	}
	
    while(device->run())
    {
        const unsigned int now = device->getTimer()->getTime();
        const double delta_time = ((now - then) / 1000.f) / AU_TIME;
        then = now;
        
        time_controller.progress(delta_time);
        
        
        if(event_receiver.isKeyDown(KEY_LEFT))
            camera_position.rotateXYBy(speed_angular * delta_time, origin);
        if(event_receiver.isKeyDown(KEY_RIGHT))
            camera_position.rotateXYBy(-speed_angular * delta_time, origin);
            
        if(event_receiver.isKeyDown(KEY_NEXT))
        {
            float cl = camera_position.getLength();
            
            if (cl - 0.2 >= 1.0)
                camera_position.setLength(camera_position.getLength() - speed_linear * delta_time);
            else
                camera_position.setLength(1.0);
        }   
        
        if(event_receiver.isKeyDown(KEY_PRIOR))
            camera_position.setLength(camera_position.getLength() + speed_linear * delta_time);
        
        if(event_receiver.isKeyDown(KEY_UP) || event_receiver.isKeyDown(KEY_DOWN))
        {
            
            float rho = camera_position.getLength();
            float theta = acos(camera_position.Z / VECT_Z / rho);
            float phi   = atan2(camera_position.Y  / VECT_Y, camera_position.X  / VECT_X);
            
            float step = speed_angular * delta_time / 180.f * M_PI;
            
            if (event_receiver.isKeyDown(KEY_UP))
            {
                if (theta - step > 0)
                    theta -= step;
            }
            else
            {
                if (theta + step < M_PI)
                    theta += step;
            }
            
            camera_position.X = rho * sin(theta) * cos(phi) * VECT_X;
            camera_position.Y = rho * sin(theta) * sin(phi) * VECT_Y;
            camera_position.Z = rho * cos(theta)  * VECT_Z;
            
        }
        
        
        if(event_receiver.isKeyDown(KEY_OEM_4))
        {
            if (render_opacity > 0)
                render_opacity--;
        }
            
        
        if(event_receiver.isKeyDown(KEY_OEM_6))
        {
            if (render_opacity < 255)
                render_opacity++;
        }

        
        if(event_receiver.isKeyDown(KEY_ADD))
        {
            if (!key_add_previous_status)
            {
                time_controller.set_speed(time_controller.get_speed() * 1.1);
            }
            key_add_previous_status = true;
        }
        else
            key_add_previous_status = false;
            
        if(event_receiver.isKeyDown(KEY_SUBTRACT ))
        {
            if (!key_sub_previous_status)
            {
                time_controller.set_speed(time_controller.get_speed() * 0.9);
            }
            key_sub_previous_status = true;
        }
        else
            key_sub_previous_status = false;
            
        if(event_receiver.isKeyDown(KEY_MULTIPLY))
        {
            if (!key_mul_previous_status)
            {
                time_controller.set_speed(time_controller.get_speed() * 10.0);
            }
            key_mul_previous_status = true;
        }
        else
            key_mul_previous_status = false;
            
        if(event_receiver.isKeyDown(KEY_DIVIDE ))
        {
            if (!key_div_previous_status)
            {
                time_controller.set_speed(time_controller.get_speed() * 0.1);  
            }
            key_div_previous_status = true;
        }
        else
            key_div_previous_status = false;
         
         
        if(event_receiver.isKeyDown(KEY_PERIOD))
        {
            if (!key_period_previous_status)
            {
                time_controller.set_play_forward();
                time_controller.play();
            }
            key_period_previous_status = true;
        }
        else
            key_period_previous_status = false;
        
        if(event_receiver.isKeyDown(KEY_COMMA))
        {
            if (!key_comma_previous_status)
            {
                time_controller.set_play_backward();
                time_controller.play();  
            }
            key_comma_previous_status = true;
        }
        else
            key_comma_previous_status = false;
            
        if(event_receiver.isKeyDown(KEY_SPACE))
        {
            if (!key_space_previous_status)
            {
                if (time_controller.is_playing())
                    time_controller.pause();
                else
                    time_controller.play();
            }
            key_space_previous_status = true;
        }
        else
            key_space_previous_status = false;
         

        if(event_receiver.isKeyDown(KEY_KEY_P))
        {
            if (!key_p_previous_status)
            {
                render_unblend = !render_unblend;
            }
            key_p_previous_status = true;
        }
        else
            key_p_previous_status = false;    
                 
        if(event_receiver.isKeyDown(KEY_KEY_Q))
        {
            if (!key_q_previous_status)
            {
				device->closeDevice();
            }
            key_q_previous_status = true;
        }
        else
            key_q_previous_status = false;         
        
        
        
        camera_node->setPosition(camera_position);
        camera_node->setTarget(origin);
        camera_node->setUpVector(vector3df(0.f * VECT_X, 0.f * VECT_Y, 1.f * VECT_Z));
        
        
        update_camera_position(text_camera, camera_position, light_node);
        
       
        
        ParticleRecord* current_record_ptr = frame_controller_interaction.get_frame(); 
        
        if (current_record_ptr != NULL)
        {
            particle_node->setVisible(true);
            ParticleRecord& current_record = *current_record_ptr;
            particle_node->setPosition(vector3df(current_record.relative_position_x * length_au_to_pixels_ratio * VECT_X, current_record.relative_position_y * length_au_to_pixels_ratio * VECT_Y, current_record.relative_position_z * length_au_to_pixels_ratio * VECT_Z));
        }
        else
            particle_node->setVisible(false);
       
       
       
        for (unsigned int i = 0; i < field_cfgs.size(); i++)
        {
            FieldMovieFrame* current_frame_ptr = frame_controllers_field[i]->get_frame();
            
            if (current_frame_ptr != NULL)
            {
                render_plane_nodes[i]->setVisible(true);
                
                if (render_unblend || render_opacity < 230)
					render_plane_nodes[i]->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
				else
					render_plane_nodes[i]->setMaterialType(EMT_SOLID);
				
                
                FieldMovieFrame& current_frame = *current_frame_ptr;
                if (!debug)
                    load_field_texture(field_cfgs[i], current_frame, field_movies[i].palette, render_opacity, render_unblend, render_plane_textures[i]);
                else
					load_debug_texture(field_cfgs[i], render_plane_textures[i]);
            }
            else
                render_plane_nodes[i]->setVisible(false);
        }
        
        
        
        driver->beginScene(true, true, SColor(255,100,101,140));
        smgr->drawAll();
        
            
        guienv->drawAll();
        driver->endScene();
        
       
        
    }
    
    free(records);
    device->drop();
    return 0;
}
