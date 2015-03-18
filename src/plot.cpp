#include <stdio.h>
#include <boost/regex.hpp>
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
	
	// Plotting position
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_x.ct2'    --output-directory '%s' --name 'position_x'")     % interaction_file.string() % output_dir.string()).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_y.ct2'    --output-directory '%s' --name 'position_y'")     % interaction_file.string() % output_dir.string()).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_z.ct2'    --output-directory '%s' --name 'position_z'")     % interaction_file.string() % output_dir.string()).str());

	// Plotting momentum
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom_x.ct2'    --output-directory '%s' --name 'momentum_x'")     % interaction_file.string() % output_dir.string()).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom_y.ct2'    --output-directory '%s' --name 'momentum_y'")     % interaction_file.string() % output_dir.string()).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom_z.ct2'    --output-directory '%s' --name 'momentum_z'")     % interaction_file.string() % output_dir.string()).str());
	
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom.ct2'      --output-directory '%s' --name 'momentum'")       % interaction_file.string() % output_dir.string()).str());

	// Plotting field on particle
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_field_x.ct2'  --output-directory '%s' --name 'field_x'")        % interaction_file.string() % output_dir.string()).str());
	
}


void plot_field_render(string axis1, string axis2, unsigned int nt, FieldRender& field_render, vector<RenderLimit>& render_limits, fs::path output_dir)
{
	for (unsigned short c = 0; c < field_render.count; c++)
	{
		string basename_global =  (bo::format("field_render_%s_%u") % field_render.id % c).str();
		
		fs::path filename_ct = output_dir / fs::path((bo::format("%s.ct2") % basename_global).str());
		
		FILE* file_ct = fopen(filename_ct.string().c_str(), "w");
		
		// Writing ctioga2 files
		fprintf(file_ct, "title '%s'\n", field_render.titles[c].c_str());
		fprintf(file_ct, "text-separator ;\n");
		fprintf(file_ct, "\n");
		fprintf(file_ct, "z_min = %.16E\n", 0.0); // TODO: add value
		fprintf(file_ct, "z_max = %.16E\n", 100.0); // TODO: add value
		fprintf(file_ct, "\n");
		fprintf(file_ct, "xyz-map\n");
		fprintf(file_ct, "new-zaxis zvalues /location right /bar_size=6mm\n");
		
		RenderLimit render_limit = render_limits[c];
		string color_range = field_render.colors[c];
		bo::replace_all(color_range, "min_abs", (bo::format("%E") % render_limit.value_min_abs).str());
		bo::replace_all(color_range, "max_abs", (bo::format("%E") % render_limit.value_max_abs).str());
		bo::replace_all(color_range, "min", 	(bo::format("%E") % render_limit.value_min).str());
		bo::replace_all(color_range, "max", 	(bo::format("%E") % render_limit.value_max).str());

		
		fprintf(file_ct, "plot @'$2:$3:$%u'  /color-map \"%s\" /zaxis zvalues\n", 4+c, color_range.c_str());
		fprintf(file_ct, "\n");
		fprintf(file_ct, "xlabel '$%s$ [$m$]'\n", axis1.c_str());
		fprintf(file_ct, "ylabel '$%s$ [$m$]'\n", axis2.c_str());
		
		fclose(file_ct);
		
		
		
		// Writing shell file that will create the movie
		fs::path filename_sh = output_dir / fs::path((bo::format("%s.sh") % basename_global).str());
		FILE* file_sh = fopen(filename_sh.string().c_str(), "w");
		fprintf(file_sh, "#!/bin/sh\n");
		fprintf(file_sh, "\n");
		
		fprintf(file_sh, "echo \"Running ctioga2 ...\"\n");
		for (unsigned int t = 0; t < nt; t++)
		{
			string basename_time = (bo::format("%s_t%u") % basename_global % t).str();
			fprintf(file_sh, "ctioga2 --text-separator \\; --load 'field_render_%s_t%u.csv' -f '%s'  --name '%s'\n", field_render.id.c_str() , t, filename_ct.filename().string().c_str(), basename_time.c_str());
		}
		fprintf(file_sh, "\n");
		
		fprintf(file_sh, "echo \"Running pdftoppm ...\"\n");
		for (unsigned int t = 0; t < nt; t++)
		{
			string basename_time = (bo::format("%s_t%u") % basename_global % t).str();
			fprintf(file_sh, (bo::format("pdftoppm -png -scale-to 1080 -singlefile '%s.pdf' '%s'\n") % basename_time % basename_time).str().c_str());
		}
		fprintf(file_sh, "\n");
		fprintf(file_sh, (bo::format("rm %s_t*.pdf\n") % basename_global).str().c_str());
		fprintf(file_sh, "\n");



		
		double framerate = ceil(nt / (field_render.movie_length * AU_TIME));
		
		fprintf(file_sh, "echo \"Running %s ...\"\n", ffmpeg_name.c_str());
		// \"
		fprintf(file_sh, "%s -framerate %.5f -loglevel error -i '%s_t%%d.png' -c:v libx264 -r 30 '%s.mp4'", ffmpeg_name.c_str(), framerate, basename_global.c_str(),/* w, h,*/ basename_global.c_str());
		fprintf(file_sh, "\n");
		fprintf(file_sh, (bo::format("rm %s_t*.png\n") % basename_global).str().c_str());
		fprintf(file_sh, "\n");
		fprintf(file_sh, "echo \"done\"\n");
		fclose(file_sh);
		
		system((bo::format("chmod a+x %s") % filename_sh.string()).str().c_str());
	}
	
}

