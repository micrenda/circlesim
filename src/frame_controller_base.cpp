#include <cstdlib>
#include <cstdio>
#include "frame_controller_base.hpp"
#include "frame_controller_interaction.hpp"
#include "time_controller.hpp"
#include "type.hpp"

template <class T> FrameControllerBase<T>::FrameControllerBase(T* arg_items, unsigned int   arg_items_size, TimeController& time_controller):
time_controller(time_controller)
{
	items 			 = arg_items;
	items_size		 = arg_items_size;
	
	movie_start      = items[0].time;
	movie_end        = items[arg_items_size-1].time;
}

template <class T> T*	FrameControllerBase<T>::get_frame()
{
	
	double current_time = time_controller.get_current_time();
	
	if (current_time < movie_start || current_time > movie_end)
		return NULL;
	
	// drawing particle
	if (time_controller.is_playing())
	{
		bool found = false;
		
		if (time_controller.is_play_forward())
		{
			// Searching particle record
			for (unsigned int i = current_item; i < items_size; i++)
			{
				if (items[i].time <=  current_time)
				{
					if (i < items_size - 1)
					{
						if (items[i+1].time > current_time)
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
					if (items[i].time <=  current_time)
					{
						if (items[i+1].time > current_time)
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
				if (items[i].time >=  current_time)
				{
					if (i > 0)
					{
						if (items[i-1].time < current_time)
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
					if (items[i].time >=  current_time)
					{
						if (items[i-1].time < current_time)
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
	
	return &items[current_item];        
}



	
// No need to call this TemporaryFunction() function, it's just to avoid link error.
template class FrameControllerBase<ParticleRecord>;
template class FrameControllerBase<FieldMovieFrame>;
