#include "time_controller_base.hpp"

#ifndef TIME_CONTROLLER_FIELD_H
#define TIME_CONTROLLER_FIELD_H

class TimeControllerField: public TimeControllerBase<FieldMovieFrame>
{		
	public:
		TimeControllerField(FieldMovieFrame* items, unsigned int items_size): TimeControllerBase<FieldMovieFrame>(items, items_size)  {}
};

#endif
