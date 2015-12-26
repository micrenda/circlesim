#include "time_controller.hpp"
#include "type.hpp"

TimeController::TimeController(double time_start, double time_end)
{
	loop_running    = true;
	loop_direction  = true;
	
	loop_start = time_start;
	loop_end   = time_end;
	
	loop_duration   = loop_end - loop_start;
	
	loop_speed     = 1.d / ((60.d / AU_TIME) / loop_duration);
	
	loop_current_time = loop_start;
}


void TimeController::progress(double delta_time)
{
	if (loop_running)
	{
		if (loop_direction)
		{
			loop_current_time += delta_time * loop_speed;
			while (loop_current_time > loop_end)
				loop_current_time -= loop_duration;
		}
		else
		{
			loop_current_time -= delta_time * loop_speed;
			while (loop_current_time < loop_start)
				loop_current_time += loop_duration;
		}
	}	
}

double TimeController::get_duration()
{
	return loop_duration;
}

double TimeController::get_current_time()
{
	return loop_current_time;
}

void TimeController::pause()
{
	loop_running = false;
}

void TimeController::play()
{
	loop_running = true;
}
		
bool TimeController::is_playing()
{
	return loop_running;
}
		
void TimeController::set_play_forward()
{
	loop_direction = true;
}

void TimeController::set_play_backward()
{
	loop_direction = false;
}

void   TimeController::set_speed(double speed)
{
	loop_speed = speed;
}

double TimeController::get_speed()
{
	return loop_speed;
}

bool TimeController::is_play_forward()
{
	return loop_direction;
}

bool TimeController::is_play_backward()
{
	return !loop_direction;
}

