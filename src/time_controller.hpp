#ifndef TIME_CONTROLLER_H
#define TIME_CONTROLLER_H

class TimeController
{
		bool   loop_running;
		bool   loop_direction; // True -> Forward, False -> Backward
		
		double loop_current_time;
		
		double loop_start;
		double loop_end;
		double loop_duration;
		double loop_speed;
		
	public:
	
		TimeController(double time_start, double time_end);
		void progress(double delta_time);
		
		void pause();
		void play();
		
		bool is_playing();
		
		bool is_play_forward();
		bool is_play_backward();
		
		void set_play_forward();
		void set_play_backward();
		
		void   set_speed(double speed);
		double get_speed();
		
		double get_duration();
		double get_current_time();
};

#endif
