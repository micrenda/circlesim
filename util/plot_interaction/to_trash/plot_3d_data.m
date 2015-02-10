function plot_3d_data(basename, output_dir, axis_labels, step, step_per_second, varargin)

    

    pos_min = +Inf;
    pos_max = -Inf;
    
    t_min = +Inf;
    t_max = -Inf;
    
    for i = 1:length(varargin)
        
        data = varargin{i};
        pos_min = min([pos_min; data(:, 2); data(:, 3); data(:, 4)]);
        pos_max = max([pos_max; data(:, 2); data(:, 3); data(:, 4)]);

        t_min = min([t_min; data(:,1)]);
        t_max =  max([t_max; data(:,1)]);
    end
    
    last_indexes = linspace(0,0,length(varargin));

    figure();
    
    current_time = t_min;
	d = 0;
	d_count = ceil((t_max - t_min) / step);
	ht = -1;
	
    while(current_time < t_max)
		

       
        for i = 1:length(varargin)
            data = varargin{i};
            buffer = [];
            start = last_indexes(i)+1;
            for line=start:length(data)
                if (data(line,1) >= current_time && data(line,1) < current_time + step)
                    buffer = [buffer; data(line,:)];
                    last_indexes(i) = line;
                end
                
                if data(line,1) >= current_time + step
                    break;
                end

            end  
            
            
            %'k'  blacK
            %'r'  Red
            %'g'  Green
            %'b'  Blue
            %'m'  Magenta
            %'c'  Cyan
            %'w'  White
            colors = {"b" "g" "m" "r" "c"};
            
            axis ([pos_min pos_max pos_min pos_max pos_min pos_max], "square", "manual");
            
            if (length(axis_labels) != 3)
                xlabel("x");
                ylabel("y");
                zlabel("z");   
            else
                xlabel(axis_labels{1});
                ylabel(axis_labels{2});
                zlabel(axis_labels{3});   
            end
            
			
            if length(buffer) > 0
                plot3(buffer(:,2), buffer(:,3), buffer(:,4), colors{mod(i-1, length(colors)) + 1});
            end
            
            #ht = text(0, pos_min * 2, pos_min, );
			#legend ({sprintf("t=%E sec", current_time)});
			#legend ({sprintf("t=%E", current_time)}, 'location', 'east');
            
            
            hold on;
            
            

        end

        print(sprintf("%s/%s_%010d.png", output_dir, basename, d++));
        printf("\rPlot %d of %d ...", d, d_count);
        
        current_time += step;
    end
    printf(" Completed\n");

    ffmpeg_bin   = "avconv";
    ffmpeg_codec = "libx264";
    ffmpeg_ext   = "mp4";

    ffmpeg_cmd = sprintf("%s -y -loglevel panic -framerate %d -i %s/%s_%%010d.png -c:v %s -r 30 %s/%s.%s", ffmpeg_bin, step_per_second, output_dir, basename, ffmpeg_codec, output_dir, basename, ffmpeg_ext);
    [status, output] = system (ffmpeg_cmd, 1);

	if status == 0
		d = 0;
		while(exist(sprintf("%s/%s_%010d.png", output_dir, basename, d), "file"	))
			delete(sprintf("%s/%s_%010d.png", output_dir, basename, d));
			d++;
		end
	else
		printf("Error calling ffmpeg command: %s\nCmd output:\n%s", ffmpeg_cmd, output);
	end

end

