#! /usr/bin/octave -qf
clear -all

arg_list = argv();

format long;



if or(nargin != 1)
    printf("\nUsage: plot <input_dir>\n\n");
else
    working_dir      = arg_list{1};
    
    

    file  = fopen(strcat(working_dir, "/", "interaction_i0_n0.csv"));
    position_data  = dlmread (file, ";", 1, 0);
    fclose(file);
    
    
    plot (position_data(:,3), position_data(:,4), ";position x;");
    xlabel("t");
    ylabel("x");

    print(strcat(working_dir, "/", "position_x.png"));
    
    
    
    
    
    
    plot (position_data(:,3), position_data(:,5), ";position y;");
    xlabel("t");
    ylabel("y");

    print(strcat(working_dir, "/", "position_y.png"));
    
    
    
    
    
    
    
    plot (position_data(:,3), position_data(:,6), ";position z;");
    xlabel("t");
    ylabel("z");

    print(strcat(working_dir, "/", "position_z.png"));
    
    
    
    
    
    
    
    
    plot (position_data(:,3), position_data(:,4), ";momentum x;");
    xlabel("t");
    ylabel("x");

    print(strcat(working_dir, "/", "momentum_x.png"));
    
    
    
    
    
    
    plot (position_data(:,3), position_data(:,5), ";momentum y;");
    xlabel("t");
    ylabel("y");
    
    print(strcat(working_dir, "/", "momentum_y.png"));
    
    
    
    
    
    
    
    plot (position_data(:,3), position_data(:,6), ";momentum z;");
    xlabel("t");
    ylabel("z");

    print(strcat(working_dir, "/", "momentum_z.png"));
end
