#include <cstdlib>
#include <cstdio>
#include "time_controller_base.hpp"
#include "time_controller_interaction.hpp"
#include "type.hpp"

template <class T> TimeControllerBase<T>::TimeControllerBase(T* arg_items, unsigned int   arg_items_size)
{
	items 			 = arg_items;
	items_size		 = arg_items_size;
	
	movie_running    = true;
	movie_direction  = true;
	
	movie_start      = items[0].time;
	movie_end        = items[arg_items_size-1].time;
	movie_duration   = movie_end - movie_start;
	movie_speed      = 4.E-14; // A good guess. Anyway we will initialize with a better value later
	
	movie_current_time = movie_start;
}

template <class T> void TimeControllerBase<T>::progress(double delta_time)
{
	if (movie_running)
	{
		if (movie_direction)
		{
			movie_current_time += delta_time * movie_speed;
			while (movie_current_time > movie_end)
				movie_current_time -= movie_duration;
		}
		else
		{
			movie_current_time -= delta_time * movie_speed;
			while (movie_current_time < movie_start)
				movie_current_time += movie_duration;
		}
	}	
}

template <class T> double TimeControllerBase<T>::get_duration()
{
	return movie_duration;
}

template <class T> double TimeControllerBase<T>::get_current_time()
{
	return movie_current_time;
}

template <class T> void TimeControllerBase<T>::pause()
{
	movie_running = false;
}

template <class T> void TimeControllerBase<T>::play()
{
	movie_running = true;
}
		
template <class T> bool TimeControllerBase<T>::is_playing()
{
	return movie_running;
}
		
template <class T> void TimeControllerBase<T>::set_play_forward()
{
	movie_direction = true;
}

template <class T> void TimeControllerBase<T>::set_play_backward()
{
	movie_direction = false;
}

template <class T> void   TimeControllerBase<T>::set_speed(double speed)
{
	movie_speed = speed;
}

template <class T> double TimeControllerBase<T>::get_speed()
{
	return movie_speed;
}

template <class T> T&	TimeControllerBase<T>::get_frame()
{
	// drawing particle
	if (movie_running)
	{
		bool found = false;
		
		if (movie_direction)
		{
			// Searching particle record
			for (unsigned int i = current_item; i < items_size; i++)
			{
				if (items[i].time <=  movie_current_time)
				{
					if (i < items_size - 1)
					{
						if (items[i+1].time > movie_current_time)
						{
							current_item = i;
							found = true;
							break;
						}
					}
					else
					{
						current_item = i;
						found = true;
						break;
					}
				}
				else
				{
					found = false;
					break;
				}
				
			}
			
			if (!found)
			{
				// Starting from the beginning
				for (unsigned int i = 0; i < current_item; i++)
				{
					if (items[i].time <=  movie_current_time)
					{
						if (items[i+1].time > movie_current_time)
						{
							current_item = i;
							found = true;
							break;
						}
					}
					else
					{
						found = false;
						break;
					}
				}
			}
		}
		else
		{
			// Searching particle record
			for (unsigned int i = current_item; i >= 0; i--)
			{
				if (items[i].time >=  movie_current_time)
				{
					if (i > 0)
					{
						if (items[i-1].time < movie_current_time)
						{
							current_item = i;
							found = true;
							break;
						}
					}
					else
					{
						current_item = i;
						found = true;
						break;
					}
				}
				else
				{
					found = false;
					break;
				}
				
			}
			
			if (!found)
			{
				// Starting from the end
				for (unsigned int i = items_size - 1; i > current_item; i--)
				{
					if (items[i].time >=  movie_current_time)
					{
						if (items[i-1].time < movie_current_time)
						{
							current_item = i;
							found = true;
							break;
						}
					}
					else
					{
						found = false;
						break;
					}
				}
			}
			
		}
		
		
		
		if (!found)
		{
			//TODO: Prepare a better error message
			printf("Unable to find the right frame sequence. [TODO: Prepare a better error message].\n");
			exit(-1);
		}
	}
	
	return items[current_item];        
}



	
// No need to call this TemporaryFunction() function, it's just to avoid link error.
template class TimeControllerBase<ParticleRecord>;
template class TimeControllerBase<FieldMovieFrame>;
