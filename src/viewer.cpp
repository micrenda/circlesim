#include <irrlicht/irrlicht.h>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <getopt.h>
#include "csv.h"


using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

using namespace std;

namespace fs = boost::filesystem;
namespace ba = boost::algorithm;
namespace bo = boost;

void print_help()
{
	std::string exe_name = "viewer";
	
	printf("Usage:\n\n");
	printf("  %s <base_directory> -i <n>\n", exe_name.c_str());
	printf("  %s -h\n", exe_name.c_str());
	printf("\n");
	printf("  -h  --help			Print this help menu\n");
	printf("  -i  --interaction 	Specify which interaction to render\n");
	printf("\n");
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

void update_camera_info(IGUIEditBox* text_camera, vector3df camera_position)
{
	text_camera->setText( (bo::wformat(L"Camera\nx=%1$+.3f, y=%2$+.3f, z=%3$+.3f\nρ=%4$+.3f, ϑ=%5$+.3f, φ=%6$+.3f") % camera_position.X % camera_position.Y % camera_position.Z % camera_position.X % camera_position.Y % camera_position.Z).str().c_str());
}




int last_fps = -1;
unsigned int then;
const float speed_linear = 15.f;
const float speed_angular = 15.f;


// This value is used to convert between real length (in meters) into pixel length
double length_conv_ratio;
// This value is used to convert between real time (in seconds) into frame time
double time_conv_ratio;


int main(int argc, char *argv[])
{
	
	fs::path base_dir 			= fs::path("");
	fs::path interaction_subdir = fs::path("");
	
	
	int current_interaction = -1;
	int current_node		= -1;

	int flag;
	static struct option long_options[] = {
		{"help",   		0, 0, 'h'},
		{"interaction",	1, 0, 'i'},
		{NULL, 0, NULL, 0}
	};
	
	int option_index = 0;
	while ((flag = getopt_long(argc, argv, "hi:", long_options, &option_index)) != -1)
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
			printf("Only one base directory can be provided\n", base_dir.c_str());
			exit(-1);
		}
    }
	
	if (base_dir == fs::path(""))
	{
		printf("Please specify the base directory.\n");
		print_help();
		exit(-1);
	}
	
	if (!fs::is_directory(base_dir))
	{
		printf("Directory '%s' does not exists\n", base_dir.c_str());
		exit(-1);
	}
		
	if (current_interaction < 0)
	{
		printf("Please specify the interaction.\n");
		print_help();
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
		print_help();
		exit(-1);
	}
	
	
	interaction_subdir = fs::path((bo::format("i%dn%d") % current_interaction % current_node).str());

	// Reading the configuration files
	
	fs::path cfg_file = base_dir / fs::path("parameters_si.cfg");
	read_config(cfg_file, simulation, laser, particle, particle_state, laboratory, field_renders, response_analyses, headers, sources);
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	

	MyEventReceiver event_receiver;
	IrrlichtDevice *device = createDevice( video::EDT_SOFTWARE, dimension2d<u32>(640, 480), 16, false, false, false, &event_receiver);

    if (!device)
        return 1;
        
    device->setWindowCaption(L"Circlesim viewer");
    
    
    IVideoDriver* driver = device->getVideoDriver();
    ISceneManager* smgr = device->getSceneManager();
    IGUIEnvironment* guienv = device->getGUIEnvironment();
    
    
    smgr->setAmbientLight(video::SColorf(0.3,0.3,0.3,1));
    
	IGUIEditBox* text_camera = guienv->addEditBox(L"Camera position: N/A",	rect<s32>(10,10,200,40), false);
	text_camera->setMultiLine(true);
	
	//IAnimatedMesh* mesh = smgr->getMesh("sydney.md2");
    //if (!mesh)
    //{
    //    device->drop();
    //    return 1;
    //}
    
    IAnimatedMesh* axis_x_mesh = smgr->addArrowMesh("x-axis", 0xFFFFA7A7, 0xFFFFA7A7,  10, 20, 10.0, 8.0, 0.3, 1.0);
    IAnimatedMesh* axis_y_mesh = smgr->addArrowMesh("y-axis", 0xFFA7FFA7, 0xFFA7FFA7,  10, 20, 10.0, 8.0, 0.3, 1.0);
    IAnimatedMesh* axis_z_mesh = smgr->addArrowMesh("z-axis", 0xFFA7A7FF, 0xFFA7A7FF,  10, 20, 10.0, 8.0, 0.3, 1.0);
     
   //ISceneNode *cube = smgr->addCubeSceneNode(15.0f, 0, -1, vector3df(150,10,10));
   //cube->setMaterialFlag(EMF_LIGHTING, false);
   // cube->setMaterialTexture( 0, driver->getTexture("sydney.bmp") );
    
    
    IMeshSceneNode* axis_x_node =  smgr->addMeshSceneNode(axis_x_mesh, 0, -1, vector3df(0, 0, 0), vector3df( 0,  0, 90));
    IMeshSceneNode* axis_y_node =  smgr->addMeshSceneNode(axis_y_mesh, 0, -1, vector3df(0, 0, 0), vector3df( 0,  0,  0));
    IMeshSceneNode* axis_z_node =  smgr->addMeshSceneNode(axis_z_mesh, 0, -1, vector3df(0, 0, 0), vector3df( 90, 0,  0));
    
    //axis_x_node->setMaterialFlag(video::EMF_LIGHTING, false);
    //axis_y_node->setMaterialFlag(video::EMF_LIGHTING, false);
    //axis_z_node->setMaterialFlag(video::EMF_LIGHTING, false);
    
    float    border_radius = 40;
    unsigned border_sides  = 100;
    SColor   border_color  = SColor(0xff, 0xff, 0xff, 0xff);
    
    float    border_side = border_radius * 2 * M_PI / border_sides;
    
    vector3df border_start_position_xy = vector3df(border_radius, 0, 0);
    vector3df border_start_position_xz = vector3df(0, 0, border_radius);
    vector3df border_start_position_yz = vector3df(0, border_radius, 0);
    
    for (int s = 0; s < border_sides; s++)
	{
		IMesh* cyl_mesh = smgr->getGeometryCreator()->createCylinderMesh(0.1, border_side, 20, border_color, false, 0.f);
		
		vector3df border_position_xy= border_start_position_xy;
		vector3df border_position_xz= border_start_position_xz;
		vector3df border_position_yz= border_start_position_yz;
		
		border_position_xy.rotateXYBy(360.f / border_sides * s, vector3df(0, 0, 0));
		border_position_xz.rotateXZBy(360.f / border_sides * s, vector3df(0, 0, 0));
		border_position_yz.rotateYZBy(360.f / border_sides * s, vector3df(0, 0, 0));
		
		smgr->addMeshSceneNode(cyl_mesh, 0, -1, border_position_xy, vector3df(0, 0, 360.f * s / border_sides ));
		smgr->addMeshSceneNode(cyl_mesh, 0, -1, border_position_xz, vector3df(-360.f * s / border_sides, 0	, 90));
		smgr->addMeshSceneNode(cyl_mesh, 0, -1, border_position_yz, vector3df(90+360.f * s / border_sides, 0, 0));
	}
    
    // Particle
    
    //ITexture * particle_texture = driver->addRenderTargetTexture(dimension2d<u32>(128, 128));
	//driver->setRenderTarget(particle_texture);
	//driver->draw2DRectangle(SColor(0xFF, 0x53, 0xA1, 0x62), rect<s32>(position2d<s32>(0,0),position2d<s32>(128,128)));
    
    IAnimatedMesh*  particle_mesh = smgr->addSphereMesh ("particle", 1.f);
    IMeshSceneNode* particle_node = smgr->addMeshSceneNode(particle_mesh, 0, -1, vector3df(0,0,0), vector3df(0,0,0));
	//particle_node->setMaterialTexture(0, particle_texture);
	
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
		
	long double particle_time;
	long double relative_position_x, relative_position_y, relative_position_z;
	long double relative_momentum_x, relative_momentum_y, relative_momentum_z;
	long double field_e_x, field_e_y, field_e_z;
	long double	field_b_x, field_b_y, field_b_z;
		
	

    
    vector3df camera_position = vector3df(0, -60, 30);
    
    ICameraSceneNode* camera_node = smgr->addCameraSceneNode(0, camera_position, axis_x_node->getAbsolutePosition());

    then = device->getTimer()->getTime();
    
    while(device->run())
    {
		const unsigned int now = device->getTimer()->getTime();
		const double delta_time = (now - then) / 1000.f;
		then = now;
		
		if(event_receiver.isKeyDown(KEY_LEFT))
            camera_position.rotateXYBy(speed_angular * delta_time, vector3df(0, 0, 0));
		if(event_receiver.isKeyDown(KEY_RIGHT))
            camera_position.rotateXYBy(-speed_angular * delta_time, vector3df(0, 0, 0));
            
        if(event_receiver.isKeyDown(KEY_PRIOR))
        {
			float cl = camera_position.getLength();
			
			if (cl - 0.2 >= 1.0)
				camera_position.setLength(camera_position.getLength() - speed_linear * delta_time);
			else
				camera_position.setLength(1.0);
		}	
		
		if(event_receiver.isKeyDown(KEY_NEXT))
            camera_position.setLength(camera_position.getLength() + speed_linear * delta_time);
		
		if(event_receiver.isKeyDown(KEY_UP) || event_receiver.isKeyDown(KEY_DOWN))
		{
			
			float rho = camera_position.getLength();
			float theta = acos(camera_position.Z / rho);
			float phi   = atan2(camera_position.Y, camera_position.X);
			
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
			
			camera_position.X = rho * sin(theta) * cos(phi);
			camera_position.Y = rho * sin(theta) * sin(phi);
			camera_position.Z = rho * cos(theta);
			
		}
		 
		update_camera_info(text_camera, camera_position);
		
		
		camera_node->setPosition(camera_position);
		camera_node->setTarget(axis_x_node->getAbsolutePosition());
		camera_node->setUpVector(vector3df(0, 0, 1));
		
		
		in.read_row(particle_time,
			relative_position_x, relative_position_y, relative_position_z,
			relative_momentum_x, relative_momentum_y, relative_momentum_z,
			field_e_x, field_e_y, field_e_z,
			field_b_x, field_b_y, field_b_z);
			
		
		// drawing particle
		particle_node->setPosition(vector3df(relative_position_x, relative_position_y, relative_position_z));
		
		
		driver->beginScene(true, true, SColor(255,100,101,140));
        smgr->drawAll();
        guienv->drawAll();
        driver->endScene();
        
       
        
	}
	
	device->drop();
    return 0;
}
