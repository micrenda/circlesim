#include "frame_controller_base.hpp"

#ifndef TIME_CONTROLLER_INTERACTION_H
#define TIME_CONTROLLER_INTERACTION_H

class ParticleRecord
{
	public:
    double time;
    double relative_position_x;
    double relative_position_y;
    double relative_position_z;
    double relative_momentum_x;
    double relative_momentum_y;
    double relative_momentum_z;
    double field_e_x;
    double field_e_y;
    double field_e_z;
    double field_b_x;
    double field_b_y;
    double field_b_z;
};

class FrameControllerInteraction: public FrameControllerBase<ParticleRecord>
{		
	public:
		FrameControllerInteraction(ParticleRecord* items, unsigned int items_size, TimeController& time_controller): FrameControllerBase<ParticleRecord>(items, items_size, time_controller)  {}
};

#endif
