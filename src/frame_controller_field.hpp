#include "frame_controller_base.hpp"

#ifndef TIME_CONTROLLER_FIELD_H
#define TIME_CONTROLLER_FIELD_H

class FrameControllerField: public FrameControllerBase<FieldMovieFrame>
{		
	public:
		FrameControllerField(FieldMovieFrame* items, unsigned int items_size, TimeController& time_controller): FrameControllerBase<FieldMovieFrame>(items, items_size, time_controller)  {}
};

#endif
