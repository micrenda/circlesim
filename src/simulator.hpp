#include <LuaState.h>
#include "type.hpp"

void simulate (
	Simulation& 				simulation,
	Pulse& 						laser,
	Particle& 					particle,
	ParticleStateGlobal& 		particle_state_global,
	Laboratory& 				laboratory,
	
	FunctionNodeEnter&        	on_node_enter,
	FunctionNodeTimeProgress& 	on_node_time_progress,
	FunctionNodeExit&         	on_node_exit,
	
	FunctionFreeEnter&        	on_free_enter,
	FunctionFreeTimeProgress& 	on_free_time_progress,
	FunctionFreeExit&         	on_free_exit,
	
	lua::State* 				lua_state);
