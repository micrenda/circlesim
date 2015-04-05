#include <stdio.h>
#include <string>
#include "type.hpp"
#include "plot.hpp"
#include "util.hpp"

extern string exe_path;
extern string exe_name;
extern string ffmpeg_name;

int execute_plot_cmd(string cmd)
{

	int status = system(cmd.c_str());
	
	if (status != 0)
	{
		
		printf("------------------------------------------------------------\n");
		printf("WARNING:\n");
		printf("Plot command returned status %d. Executed command: %s\n", status, cmd.c_str());
		printf("------------------------------------------------------------\n");
		
		exit(-6);
	}
	
	return status;
}

void plot_interaction_files(fs::path output_dir)
{
	
	fs::path interaction_file   = output_dir / fs::path("interaction.csv");
	fs::path field_file         = output_dir / fs::path("field.csv");
	
	#pragma omp parallel sections shared(interaction_file, output_dir)
	{
	// Plotting position
		#pragma omp section
		{execute_plot_cmd((bo::format("ctioga2 --no-mark --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_x.ct2'    --output-directory '%s' --name 'position_x'")     % interaction_file.string() % output_dir.string()).str());}
		#pragma omp section
		{execute_plot_cmd((bo::format("ctioga2 --no-mark --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_y.ct2'    --output-directory '%s' --name 'position_y'")     % interaction_file.string() % output_dir.string()).str());}
		#pragma omp section
		{execute_plot_cmd((bo::format("ctioga2 --no-mark --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_z.ct2'    --output-directory '%s' --name 'position_z'")     % interaction_file.string() % output_dir.string()).str());}
		
		#pragma omp section
		{execute_plot_cmd((bo::format("ctioga2 --no-mark --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_xy.ct2'    --output-directory '%s' --name 'position_xy'")     % interaction_file.string() % output_dir.string()).str());}
		#pragma omp section
		{execute_plot_cmd((bo::format("ctioga2 --no-mark --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_xz.ct2'    --output-directory '%s' --name 'position_xz'")     % interaction_file.string() % output_dir.string()).str());}
		#pragma omp section
		{execute_plot_cmd((bo::format("ctioga2 --no-mark --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_yz.ct2'    --output-directory '%s' --name 'position_yz'")     % interaction_file.string() % output_dir.string()).str());}

		// Plotting momentum
		#pragma omp section
		{execute_plot_cmd((bo::format("ctioga2 --no-mark --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom_x.ct2'    --output-directory '%s' --name 'momentum_x'")     % interaction_file.string() % output_dir.string()).str());}
		#pragma omp section
		{execute_plot_cmd((bo::format("ctioga2 --no-mark --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom_y.ct2'    --output-directory '%s' --name 'momentum_y'")     % interaction_file.string() % output_dir.string()).str());}
		#pragma omp section
		{execute_plot_cmd((bo::format("ctioga2 --no-mark --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom_z.ct2'    --output-directory '%s' --name 'momentum_z'")     % interaction_file.string() % output_dir.string()).str());}
		
		#pragma omp section
		{execute_plot_cmd((bo::format("ctioga2 --no-mark --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom.ct2'      --output-directory '%s' --name 'momentum'")       % interaction_file.string() % output_dir.string()).str());}

		// Plotting field on particle
		#pragma omp section
		{execute_plot_cmd((bo::format("ctioga2 --no-mark --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_field_x.ct2'  --output-directory '%s' --name 'field_x'")        % interaction_file.string() % output_dir.string()).str());}
	}
}

void plot_labmap(fs::path output_dir, short axis_1, short axis_2)
{
	string axis_1_label;
	string axis_2_label;
	
	switch (axis_1)
	{
		case 1:
			axis_1_label = "x";
		break;
		case 2:
			axis_1_label = "y";
		break;
		case 3:
			axis_1_label = "z";
		break;
	}
	switch (axis_2)
	{
		case 1:
			axis_2_label = "x";
		break;
		case 2:
			axis_2_label = "y";
		break;
		case 3:
			axis_2_label = "z";
		break;
	}
	
	
	fs::path input_file_mask = output_dir / fs::path((bo::format("labmap_%s%s_t%%d.png") % axis_1_label % axis_2_label).str());
	fs::path output_file     = output_dir / fs::path((bo::format("labmap_%s%s.mp4") % axis_1_label % axis_2_label).str());
	
	string ffmpeg_cmd = (bo::format("%s -loglevel error -framerate 5 -i %s -c:v libx264 -r 30 %s") % ffmpeg_name % input_file_mask % output_file.string()).str();	
	
	int result = execute_plot_cmd(ffmpeg_cmd);
	
	if (result == 0)
	{
		fs::path delete_file_mask =output_dir / fs::path((bo::format("labmap_%s%s_t*.png") % axis_1_label % axis_2_label).str());
		system((bo::format("rm %s") % delete_file_mask.string()).str().c_str());
	}
}
