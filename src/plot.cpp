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
	
	// Plotting position
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_x.ct2'    --output-directory '%s' --name 'position_x'")     % interaction_file.string() % output_dir.string()).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_y.ct2'    --output-directory '%s' --name 'position_y'")     % interaction_file.string() % output_dir.string()).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_z.ct2'    --output-directory '%s' --name 'position_z'")     % interaction_file.string() % output_dir.string()).str());
	
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_xy.ct2'    --output-directory '%s' --name 'position_xy'")     % interaction_file.string() % output_dir.string()).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_xz.ct2'    --output-directory '%s' --name 'position_xz'")     % interaction_file.string() % output_dir.string()).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_pos_yz.ct2'    --output-directory '%s' --name 'position_yz'")     % interaction_file.string() % output_dir.string()).str());

	// Plotting momentum
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom_x.ct2'    --output-directory '%s' --name 'momentum_x'")     % interaction_file.string() % output_dir.string()).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom_y.ct2'    --output-directory '%s' --name 'momentum_y'")     % interaction_file.string() % output_dir.string()).str());
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom_z.ct2'    --output-directory '%s' --name 'momentum_z'")     % interaction_file.string() % output_dir.string()).str());
	
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_mom.ct2'      --output-directory '%s' --name 'momentum'")       % interaction_file.string() % output_dir.string()).str());

	// Plotting field on particle
	execute_plot_cmd((bo::format("ctioga2 --text-separator \\; --load '%s' -f 'util/plot_interaction/interaction_field_x.ct2'  --output-directory '%s' --name 'field_x'")        % interaction_file.string() % output_dir.string()).str());
	
}



