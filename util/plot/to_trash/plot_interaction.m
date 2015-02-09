#! /usr/bin/octave -qf
clear -all
addpath(".");

arg_list = argv();



if or(nargin < 1, nargin > 3)
    printf("\nUsage: plot <input_dir> <slow_rate>\n\n");
else
    working_dir      = arg_list{1};
    
    fps = 25;
    slow_rate = 1000
    
    if nargin >= 2
		slow_rate = str2double(arg_list{2})
	end
	

	frame_duration	 =  1.0 / fps / slow_rate;
          
    file  = fopen(strcat(working_dir, "/", "interaction.csv"));
    interaction  = dlmread (file, ";", 1, 0);
    fclose(file);
    
    file  = fopen(strcat(working_dir, "/", "field.csv"));
    field  = dlmread (file, ";", 1, 0);
    fclose(file);

    mkdir(working_dir);
    
    scale_e_field = 1E10;
    field(:,7:9) = field(:,7:9) / scale_e_field;
    interaction(:,16:18) = interaction(:,16:18) / scale_e_field;
    
    plot_3d_data("interaction", working_dir, {"x","y","z"},  frame_duration, fps, interaction(:,3:6), field(:,[3,7:9]), interaction(:,[3,16:18]));
end
