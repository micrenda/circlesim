#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <libgen.h>
#include <LuaState.h>
#include "main.hpp"
#include "type.hpp"
#include "config.hpp"
#include "simulator.hpp"
#include "output.hpp"

extern string exe_path;
extern string exe_name;

void print_help()
{
	string exe_name = "circlesim";
	
    printf("Usage:\n\n");
    printf("  %s -c <config_file.cfg> [-o <output_dir>]\n", exe_name.c_str());
    printf("  %s -h\n", exe_name.c_str());
    printf("\n");
    printf("  -o  --output <output_dir>                Set output directory (default /tmp)\n");
    printf("  -c  --config <config_file>               Set configuration file\n");
    printf("  -h  --help                               Print this help menu\n");
    printf("\n");
}

int lua_error_handler(lua_State* L)
{
   lua_Debug d;
   lua_getstack(L, 1, &d);
   lua_getinfo(L, "Sln", &d);
   std::string err = lua_tostring(L, -1);
   lua_pop(L, 1);
   std::stringstream msg;
   msg << d.short_src << ":" << d.currentline;

   if (d.name != 0)
   {
      msg << "(" << d.namewhat << " " << d.name << ")";
   }
   msg << " " << err;
   lua_pushstring(L, msg.str().c_str());
   return 1;
}


int main(int argc, char *argv[])
{
	
	// Reading executable exe location
	char exe_fullpath[PATH_MAX];
	readlink("/proc/self/exe", exe_fullpath, sizeof(exe_fullpath)-1);
	exe_path = string(dirname(exe_fullpath));
	exe_name = string(basename(exe_fullpath));
	
	
	
	fs::path cfg_file_orig = fs::path("");
	fs::path base_dir = fs::current_path();

	int flag;
	static struct option long_options[] = {
		{"help", 	0, 0, 'h'},
		{"output", 	1, 0, 'o'},
		{"config", 	1, 0, 'c'},
		{NULL, 0, NULL, 0}
	};
	
	int option_index = 0;
	while ((flag = getopt_long(argc, argv, "ho:c:", long_options, &option_index)) != -1)
	{
		switch (flag)
		{
		case 'o':
			base_dir = optarg;
			break;
		case 'c':
			cfg_file_orig = fs::path(optarg);
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
	
	
	if (cfg_file_orig == fs::path(""))
	{
		printf("Please specify the configuration filename using the -c flag\n\n");
		exit(-1);
	}

	
	
	if (!fs::is_regular_file(cfg_file_orig))
	{
		printf("Unable to read file '%s'\n", cfg_file_orig.c_str());
		exit(-1);
	}
	
	// If we does not specify the output path, then we out our output in a subdirectory
	if (base_dir == fs::path(exe_path))
		base_dir /= fs::path("output");
	
	
	// Create a new lua state
	lua::State		lua_state;

	
	Simulation		simulation;
	Pulse			laser;
	Particle		particle;
	Accellerator 	accellerator;
	Node*			nodes = NULL; // Array of nodes
	ParticleState	particle_state;
	OutputSetting   output_setting;
	
	// Convert the config file to a SI compliant version
	fs::path cfg_file_si_tmp = fs::temp_directory_path() / fs::unique_path();
	string convert_cmd = (bo::format("java -jar '%s/util/unit/ConfigUnitConvertor.jar' --convert-cfg SI '%s/util/unit/conversions.csv' '%s' '%s'") % exe_path % exe_path % cfg_file_orig.string() % cfg_file_si_tmp.string()).str();
	int convert_status = system(convert_cmd.c_str());
	
	if (convert_status != 0)
	{
		printf("ERROR - There was a problem when converting the units to SI system. Please check the output above.\n");
		printf("Executed cmd: %s\n", convert_cmd.c_str());
		exit(-2);
	}
	
	if(true)
	{
		// Please don't use the parameters variable outside this block
		Parameters parameters;
	
		read_config(cfg_file_si_tmp, parameters, simulation, output_setting, laser, particle, particle_state, accellerator, nodes, &lua_state);
		
		nodes = new Node[accellerator.nodes];
		init_nodes(accellerator, nodes);
		init_position_and_momentum(parameters, accellerator, particle_state, nodes);
	}
	
	
	// Creating output directory
	fs::path output_dir;
	
	unsigned int i = 1;
	do
	{
		// Try to find a name for the output folder. It increase the suffix until is finds that does not exists a folder with that name
		output_dir = base_dir / fs::path((bo::format("%s_%d") % simulation.basename % i++).str());
	}
	while(fs::exists(output_dir));
	

	printf("Creating directory '%s'\n", output_dir.c_str());
	fs::create_directories(output_dir);
	fs::copy_file(cfg_file_orig,   output_dir / fs::path("parameters_orig.cfg"));
	
	fs::path cfg_file_si = output_dir / fs::path("parameters_si.cfg");
	fs::copy_file(cfg_file_si_tmp, cfg_file_si);
	fs::remove(fs::path(cfg_file_si_tmp));
	
	// Append to README.txt the conversion units used
	string units_txt_cmd = (bo::format("java -jar '%s/util/unit/ConfigUnitConvertor.jar' --print-units SI '%s/util/unit/conversions.csv' >> %s/README.txt") %  exe_path % output_dir % exe_path).str();
	system(units_txt_cmd.c_str());
	
	
	// Opening output files
	open_global_files(output_dir);
	
	// Writing nodes
	for (unsigned int n = 0; n < accellerator.nodes; n++)
		write_node(nodes[n]);
	
	
	// Executing simulation
	simulate(simulation, output_setting, laser, particle, particle_state, accellerator, nodes, &lua_state, output_dir);
	
	// Close output files
	close_global_files();
}
