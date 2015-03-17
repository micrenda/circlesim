#include <stdio.h>
#include <boost/regex.hpp>
#include <string>
#include "type.hpp"
#include "plot.hpp"

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

void plot_interaction_files(unsigned int interaction, int node, fs::path output_dir)
{
	fs::path plot_dir = output_dir / fs::path("plot_interactions");
	
	// Creating plot directory, if does not exists
	create_directory(plot_dir);
	
	fs::path interaction_file   = output_dir / fs::path((bo::format("interaction_i%un%d.csv") % interaction % node).str());
	fs::path field_file         = output_dir / fs::path((bo::format("field_i%un%d.csv")       % interaction % node).str());
	
	// Plotting position
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_x.ct2'    --output-directory '%s' --name 'position_x_i%un%d'")     % interaction_file.string() % plot_dir.string() % interaction % node).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_y.ct2'    --output-directory '%s' --name 'position_y_i%un%d'")     % interaction_file.string() % plot_dir.string() % interaction % node).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_z.ct2'    --output-directory '%s' --name 'position_z_i%un%d'")     % interaction_file.string() % plot_dir.string() % interaction % node).str());

	// Plotting momentum
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom_x.ct2'    --output-directory '%s' --name 'momentum_x_i%un%d'")     % interaction_file.string() % plot_dir.string() % interaction % node).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom_y.ct2'    --output-directory '%s' --name 'momentum_y_i%un%d'")     % interaction_file.string() % plot_dir.string() % interaction % node).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom_z.ct2'    --output-directory '%s' --name 'momentum_z_i%un%d'")     % interaction_file.string() % plot_dir.string() % interaction % node).str());
	
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom.ct2'      --output-directory '%s' --name 'momentum_i%un%d'")       % interaction_file.string() % plot_dir.string() % interaction % node).str());

	// Plotting field on particle
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_field_x.ct2'  --output-directory '%s' --name 'field_x_i%un%d'")        % interaction_file.string() % plot_dir.string() % interaction % node).str());
	
}



void make_field_map_video(fs::path output_dir, fs::path file_ct, RenderLimit& render_limits, string plane, unsigned int interaction, int node, unsigned int max_t, double movie_length)
{
	string basename_ct = file_ct.stem().string();
	
	// Creating plot directory, if does not exists
	fs::path plot_dir = output_dir / fs::path("plot_field_maps");
	fs::create_directory(plot_dir);
	
	
	
	#pragma omp parallel for
	for (unsigned int t = 0; t <= max_t; t++)
	{
		string basename_in  = (bo::format("field_map_%s_i%un%it%u") % plane 	  % interaction % node % t).str();
		string basename_out = (bo::format("%s_i%un%it%u") 			% basename_ct % interaction % node % t).str();

		execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s/%s.csv' -f '%s'  --output-directory '%s' --name '%s'")
			% output_dir.string()
			% basename_in
			% file_ct.string()
			% plot_dir.string() 
			% basename_out).str());

		execute_plot_cmd((bo::format("pdftoppm -singlefile '%s/%s.pdf' '%s/%s'") % plot_dir.string() % basename_out % plot_dir.string() % basename_out).str());

		
	}
	
	double framerate = ceil((1 + max_t) / (movie_length * AU_TIME));
	
	
	
	string basename_ffmpeg_input  =  (bo::format("%s_i%un%it%%d") 			% basename_ct % interaction % node).str();
	string basename_ffmpeg_output =  (bo::format("%s_i%un%it") 				% basename_ct % interaction % node).str();
	
	int ffmpeg_status = execute_plot_cmd((bo::format("%s -framerate %.5f -loglevel quiet -i '%s/%s.ppm' -vf \"scale=trunc(iw/2)*2:trunc(ih/2)*2\" -c:v libx264 -r 30 '%s/%s.mp4'")
		% ffmpeg_name
		% framerate
		% plot_dir.string() 
		% basename_ffmpeg_input
		% plot_dir.string() 
		% basename_ffmpeg_output).str());
	
	if (ffmpeg_status == 0)
	{
		for (unsigned int t = 0; t <= max_t; t++)
		{
			string basename_out = (bo::format("%s_i%un%it%u") 			% basename_ct % interaction % node % t).str();
			fs::remove(plot_dir / fs::path((bo::format("%s.ppm") % basename_out).str()));
			fs::remove(plot_dir / fs::path((bo::format("%s.pdf") % basename_out).str()));
		}
	}
}


void plot_field_maps( FieldRender& render, RenderLimit& render_limits, unsigned int interaction, int node, fs::path output_dir)
{
	
	
	//make_field_map_video(output_dir, current_iter->path(), limits, "yz", interaction, node, max_t_yz, output_setting.field_map_movie_length);
}

