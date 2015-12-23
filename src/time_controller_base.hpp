#ifndef TIME_CONTROLLER_BASE_H
#define TIME_CONTROLLER_BASE_H

template <class T>
class TimeControllerBase
{
	private:
		TimeControllerBase() { }
		
		T* items;
		unsigned int    items_size;
		
		unsigned int    current_item;
		
		bool   movie_running;
		bool   movie_direction; // True -> Forward, False -> Backward
		
		double movie_current_time;
		
		double movie_start;
		double movie_end;
		double movie_duration;
		double movie_speed;
		
	public:
		TimeControllerBase(T* items, unsigned int    items_size);
		
		T&	get_frame();
		void progress(double delta_time);
		
		void pause();
		void play();
		
		bool is_playing();
		
		void set_play_forward();
		void set_play_backward();
		
		void   set_speed(double speed);
		double get_speed();
		
		double get_duration();
		double get_current_time();
};

#endif
