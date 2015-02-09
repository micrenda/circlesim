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
	}
	
	return status;
}

void plot_interaction_files(fs::path output_dir, unsigned int interaction, int node)
{
	fs::path plot_dir = output_dir / fs::path("plot_interactions");
	
	// Creating plot directory, if does not exists
	create_directory(plot_dir);
	
	fs::path interaction_file   = output_dir / fs::path((bo::format("interaction_i%un%d.csv") % interaction % node).str());
	fs::path field_file         = output_dir / fs::path((bo::format("field_i%un%d.csv")       % interaction % node).str());
	
	// Plotting position
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot/interaction_pos_x.ct2'    --output-directory '%s' --name 'position_x_i%un%d'")     % interaction_file.string() % plot_dir.string() % interaction % node).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot/interaction_pos_y.ct2'    --output-directory '%s' --name 'position_y_i%un%d'")     % interaction_file.string() % plot_dir.string() % interaction % node).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot/interaction_pos_z.ct2'    --output-directory '%s' --name 'position_z_i%un%d'")     % interaction_file.string() % plot_dir.string() % interaction % node).str());

	// Plotting momentum
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot/interaction_mom_x.ct2'    --output-directory '%s' --name 'momentum_x_i%un%d'")     % interaction_file.string() % plot_dir.string() % interaction % node).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot/interaction_mom_y.ct2'    --output-directory '%s' --name 'momentum_y_i%un%d'")     % interaction_file.string() % plot_dir.string() % interaction % node).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot/interaction_mom_z.ct2'    --output-directory '%s' --name 'momentum_z_i%un%d'")     % interaction_file.string() % plot_dir.string() % interaction % node).str());
	
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot/interaction_mom.ct2'      --output-directory '%s' --name 'momentum_i%un%d'")       % interaction_file.string() % plot_dir.string() % interaction % node).str());

	// Plotting field on particle
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot/interaction_field_x.ct2'  --output-directory '%s' --name 'field_x_i%un%d'")        % interaction_file.string() % plot_dir.string() % interaction % node).str());
	
}



void make_field_map_video(fs::path output_dir, string plane, unsigned int interaction, int node, unsigned int max_t)
{
	// Creating plot directory, if does not exists
	fs::path plot_dir = output_dir / fs::path("plot_field_maps");
	fs::create_directory(plot_dir);
	
	for (unsigned int t = 0; t <= max_t; t++)
	{
		string basename = (bo::format("field_map_%s_i%un%it%u") % plane % interaction % node % t).str();
		
		execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s/%s.csv' --set z-min 0 --set z-max 0.2 -f '%s/util/plot/interaction_field_map_%s.ct2'  --output-directory '%s' --name '%s'")
			% output_dir.string()
			% basename
			% exe_path
			% plane
			% plot_dir.string() 
			% basename).str());
		
		execute_plot_cmd((bo::format("pdftoppm -singlefile '%s/%s.pdf' '%s/%s'") % plot_dir.string() % basename % plot_dir.string() % basename).str());

		
	}
	
	int ffmpeg_status = execute_plot_cmd((bo::format("%s -framerate 4 -i '%s/field_map_%s_i%un%it%%d.ppm' -vf \"scale=trunc(iw/2)*2:trunc(ih/2)*2\" -c:v libx264 -r 30 '%s/field_map_%s_i%un%i.mp4'")
		% ffmpeg_name
		% plot_dir.string() 
		% plane
		% interaction 
		% node 
		% plot_dir.string() 
		% plane
		% interaction 
		% node).str());
	
	if (ffmpeg_status == 0)
	{
		for (unsigned int t = 0; t <= max_t; t++)
		{
			fs::remove(plot_dir / fs::path((bo::format("field_map_%s_i%un%it%u.ppm") % plane % interaction % node % t).str()));
			fs::remove(plot_dir / fs::path((bo::format("field_map_%s_i%un%it%u.pdf") % plane % interaction % node % t).str()));
		}
	}
}


void plot_field_maps(fs::path output_dir, unsigned int interaction, int node)
{
	
	fs::directory_iterator end_iter;

	if ( fs::exists(output_dir) && fs::is_directory(output_dir))
	{
		static const bo::regex e("^field\\_map\\_([xyz]{2})\\_i([0-9]+)n([0-9]+)t([0-9]+).csv$");

		bool has_xy = false;
		bool has_xz = false;
		bool has_yz = false;
		
		unsigned int max_t_xy = 0;
		unsigned int max_t_xz = 0;
		unsigned int max_t_yz = 0;
		
		for( fs::directory_iterator current_iter(output_dir); current_iter != end_iter ; ++current_iter)
		{
			if (fs::is_regular_file(current_iter->status()) )
			{
				bo::match_results<string::const_iterator> what;
				if (bo::regex_match(current_iter->path().filename().string(), what, e))
				{
					string filename = what[0];
					string plane    = what[1];
					
					char* bufc;
					unsigned int i  = strtol(what[2].str().c_str(), &bufc, 10); // In C++11 replace with stol
					int n           = strtol(what[3].str().c_str(), &bufc, 10);
					unsigned int t  = strtol(what[4].str().c_str(), &bufc, 10);
					
					if (interaction == i && node == n)
					{
						if (plane == "xy")
						{
							has_xy = true;
							if (t > max_t_xy) max_t_xy = t;
						}
						if (plane == "xz")
						{
							has_xz = true;
							if (t > max_t_xz) max_t_xz = t;
						}
						if (plane == "yz")
						{
							has_yz = true;
							if (t > max_t_yz) max_t_yz = t;
						}
					}
					
				} 
			}
		}
		
		if (has_xy) make_field_map_video(output_dir, "xy", interaction, node, max_t_xy);
		if (has_xz) make_field_map_video(output_dir, "xz", interaction, node, max_t_xz);
		if (has_yz) make_field_map_video(output_dir, "yz", interaction, node, max_t_yz);
	}
}

