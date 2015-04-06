#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <libgen.h>
#include <omp.h>
#include <dlfcn.h>
#include "main.hpp"
#include "type.hpp"
#include "plot.hpp"
#include "config.hpp"
#include "simulator.hpp"
#include "output.hpp"
#include "response.hpp"
#include "labmap.hpp"
#include "script.hpp"

extern string exe_path;
extern string exe_name;
extern string ffmpeg_name;

void print_help()
{
	string exe_name = "circlesim";
	
	printf("Usage:\n\n");
	printf("  %s -c <config_file.cfg> [-o <output_dir>] [-j <num_threads>]\n", exe_name.c_str());
	printf("  %s -h\n", exe_name.c_str());
	printf("\n");
	printf("  -o  --output  <output_dir>                Set output directory (default /tmp)\n");
	printf("  -c  --config  <config_file>               Set configuration file\n");
	printf("  -j  --threads <num_threads>               Set how many threads to use (default 1)\n");
	printf("  -h  --help                                Print this help menu\n");
	printf("\n");
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
	int num_threads = 1;

	int flag;
	static struct option long_options[] = {
		{"help",    0, 0, 'h'},
		{"output",  1, 0, 'o'},
		{"threads", 1, 0, 'j'},
		{"config",  1, 0, 'c'},
		{NULL, 0, NULL, 0}
	};
	
	int option_index = 0;
	while ((flag = getopt_long(argc, argv, "ho:c:j:", long_options, &option_index)) != -1)
	{
		switch (flag)
		{
		case 'o':
			base_dir = optarg;
			break;
		case 'c':
			cfg_file_orig = fs::path(optarg);
			break;
		case 'j':
			num_threads = stoi(optarg);
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
		printf("Please specify the configuration filename using the -c flag\n");
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
	
	
	// Checking which ffmpeg command is available (Debian uses avconv while other distros use ffmpeg)
	if (system("avconv -h > /dev/null 2> /dev/null") == 0)
	{
		ffmpeg_name = "avconv";
	}
	else if (system("ffmpeg -h > /dev/null 2> /dev/null") == 0)
	{
		ffmpeg_name = "ffmpeg";
	}
	else
	{
		ffmpeg_name = "ffmpeg";
		printf("WARNING: Unable to find 'ffmpeg' command. The video creation will be disabled.\n"); 
	}
	
	if (num_threads >= 1)
	{
		omp_set_num_threads(num_threads);
		
		printf("Max number of threads: %d.\n", num_threads);
	}
	else
	{
		printf("The <num_threads> parameters specified with option -j must be not smaller than 1.\n");
		exit(-1);
	}

			

	Simulation          simulation;
	Pulse               laser;
	Particle            particle;
	Laboratory                  laboratory;
	ParticleStateGlobal         particle_state;
	ParticleStateGlobal         particle_state_initial;
	vector<FieldRender>         field_renders;
	vector<ResponseAnalysis>    response_analyses;
	
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
	
	vector<string> headers;
	vector<string> sources;
	
	read_config(cfg_file_si_tmp, simulation, laser, particle, particle_state, laboratory, field_renders, response_analyses, headers, sources);
	
	particle_state_initial = particle_state;
	
	// Creating output directory
	fs::path output_dir;
	fs::path output_interaction_dir;
	
	unsigned int i = 1;
	do
	{
		// Try to find a name for the output folder. It increase the suffix until is finds that does not exists a folder with that name
		output_dir = base_dir / fs::path((bo::format("%s_run%d") % simulation.basename % i++).str());
	}
	while(fs::exists(output_dir));
	

	printf("Creating directory '%s'\n", output_dir.c_str());
	fs::create_directories(output_dir);
	fs::copy_file(cfg_file_orig,   output_dir / fs::path("parameters_orig.cfg"));
	
	fs::path cfg_file_si = output_dir / fs::path("parameters_si.cfg");
	fs::copy_file(cfg_file_si_tmp, cfg_file_si);
	fs::remove(fs::path(cfg_file_si_tmp));
	
	// Append to README.txt the conversion units used
	string units_txt_cmd = (bo::format("java -jar '%s/util/unit/ConfigUnitConvertor.jar' --print-units SI '%s/util/unit/conversions.csv' >> %s/README.txt") %  exe_path % exe_path % output_dir.string()).str();
	system(units_txt_cmd.c_str());
	
	ofstream stream_node;
	stream_node.open(get_filename_node(output_dir));
	setup_node(stream_node);
	for (Node& node: laboratory.nodes)
		write_node(stream_node, node);
	stream_node.close();
	
	
	// Building auxiliary library
	build_auxiliary_library(headers, sources, output_dir);
	
	fs::path shared_lib = output_dir / fs::path("custom_scripts.so");
	void* custom_lib = dlopen(shared_lib.string().c_str(), RTLD_NOW);

	if (!custom_lib)
	{
		printf("ERROR - Unable to open shared library at position '%s'\n", shared_lib.string().c_str());
		exit(-5);
	}
	
	char* error;
	if ((error = dlerror()) != NULL)
	{
		printf("ERROR - Error during %s loading: %s\n", "custom_scripts.so", error);
		exit(-5);
	}
	
	FunctionFieldType function_field = (FunctionFieldType) dlsym(custom_lib, "field");
	
	if ((error = dlerror()) != NULL)
	{
		printf("ERROR - Error during dynamic function '%s' loading: %s\n", "field", error);
		exit(-5);
	}
	
	for (FieldRender render: field_renders)
	{
		string function_name = (bo::format("func_field_render_%s") % render.id).str();
		FunctionRenderType function_render = (FunctionRenderType) dlsym(custom_lib, function_name.c_str());
		
		if (function_render)
			render.function_render = function_render;
		else
		{
			printf("ERROR - Unable to load the function '%s'\n", function_name.c_str());
			exit(-5);
		}
		
		if ((error = dlerror()) != NULL)
		{
			printf("ERROR - Error during dynamic function '%s' loading: %s\n", function_name.c_str(), error);
			exit(-5);
		}
	}
	
	// Setting up particle stream
	ofstream stream_particle;
	ofstream stream_interaction;

	stream_particle.open(get_filename_particle(output_dir));
	setup_particle(stream_particle);
	
	FunctionNodeEnter        on_node_enter          = [&](Simulation& simulation, Pulse& laser, Particle& particle, ParticleStateLocal&  particle_state, unsigned int current_interaction, Node& node, double time_local) mutable 
	{
		output_interaction_dir = output_dir / fs::path((bo::format("i%un%u") % current_interaction % node.id).str());
		fs::create_directories(output_interaction_dir);
		stream_interaction.open(get_filename_interaction(output_interaction_dir));
		setup_interaction(stream_interaction);
	};
	
	FunctionNodeTimeProgress on_node_time_progress  = [&](Simulation& simulation, Pulse& laser, Particle& particle, ParticleStateLocal&  particle_state, unsigned int current_interaction, Node& node, double time_local, Field& field) mutable
	{
		printf("\rSimulating node: %+.16f (i%un%u)", time_local * AU_TIME, current_interaction, node.id);
		fflush(stdout);
		write_interaction(stream_interaction, time_local, particle_state, field);
	};
	
	FunctionNodeExit         on_node_exit           = [&](Simulation& simulation, Pulse& laser, Particle& particle, ParticleStateLocal&  particle_state, unsigned int current_interaction, Node& node, double time_local) mutable
	{
		printf("\n");
		
		stream_interaction.close();
		plot_interaction_files(output_interaction_dir);
		
		// Creating field renders
		for (FieldRender field_render: field_renders)
		{
			if (field_render.enabled)
			{
				FieldRenderResult field_render_result;
				calculate_field_map(field_render_result, field_render, current_interaction, node.id,  laser, *function_field, output_interaction_dir);
			}
		}
	};
	
	FunctionFreeEnter        on_free_enter          = [&](Simulation& simulation, Particle& particle, ParticleStateGlobal& particle_state, Laboratory& laboratory, long double time_global) mutable
	{
		write_particle(stream_particle, time_global, particle_state);
	};
	
	FunctionFreeTimeProgress on_free_time_progress  = [&](Simulation& simulation, Particle& particle, ParticleStateGlobal& particle_state, Laboratory& laboratory, long double time_global) mutable
	{
		printf("\rSimulating free: %3.4f%%", (double) time_global / simulation.duration * 100);
		fflush(stdout);
		
		write_particle(stream_particle, time_global, particle_state);
		
	};
	
	FunctionFreeExit         on_free_exit           = [&](Simulation& simulation, Particle& particle, ParticleStateGlobal& particle_state, Laboratory& laboratory, long double time_global) mutable
	{
		write_particle(stream_particle, time_global, particle_state);
		printf("\n");
		
	};
	
	
	vector<SimluationResultFreeSummary> summaries_free;
	vector<SimluationResultNodeSummary> summaries_node;
	
	// Executing main simulation
	simulate (simulation, laser, particle, particle_state, laboratory,
				on_node_enter, on_node_time_progress, on_node_exit,
				on_free_enter, on_free_time_progress, on_free_exit,
				summaries_free, summaries_node,
				*function_field);
	
	stream_particle.close();
	
	#pragma omp parallel sections shared(laboratory, simulation, laser, summaries_free, summaries_node,output_dir)
	{
		#pragma omp section
		{
				render_labmap(laboratory, simulation, laser, summaries_free, summaries_node, 1, 2, *function_field, output_dir);
		}
		
		#pragma omp section 
		{
				render_labmap(laboratory, simulation, laser, summaries_free, summaries_node, 1, 3, *function_field, output_dir);
		}
		
		#pragma omp section 
		{
				render_labmap(laboratory, simulation, laser, summaries_free, summaries_node, 2, 3, *function_field, output_dir);
		}
	}
	
	// Executing response analyses simulations
	
	for (unsigned int a = 0; a < response_analyses.size(); a++)
	{
		printf("\rResponse analisys %u: %u/%u", analisys.id, 0, response_analyses[a].change_steps);
		
		FunctionResponseAnalysisCalculated on_calculate          = [&](ResponseAnalysis& analisys, unsigned int step) mutable
		{
			printf("\rResponse analisys %u: %u/%u", analisys.id, step + 1, analisys.change_steps);
		};
		
		calculateResponseAnalyses(response_analyses[a], simulation, particle, particle_state_initial, particle_state, laser, laboratory, output_dir, *function_field, on_calculate);
		
		printf("\n");
	}
	
	
	dlclose(custom_lib);

}
