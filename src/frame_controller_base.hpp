#ifndef TIME_CONTROLLER_BASE_H
#define TIME_CONTROLLER_BASE_H

#include "time_controller.hpp"

template <class T>
class FrameControllerBase
{
	private:
		FrameControllerBase() = delete;
		
		T* items;
		unsigned int    items_size;
		unsigned int    current_item;
		
		double movie_start;
		double movie_end;
		
		TimeController& time_controller;

		
	public:
		FrameControllerBase(T* items, unsigned int    items_size, TimeController& time_controller);
		
		T*	get_frame();
		
};

#endif
