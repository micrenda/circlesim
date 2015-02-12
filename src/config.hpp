#include <LuaState.h>
#include "type.hpp"

void read_config(fs::path& cfg_file, Simulation& simulation, OutputSetting& output, Pulse& laser, Particle& particle, ParticleState& particle_state, Accellerator& accellerator, lua::State* lua_state);
//void init_nodes(Accellerator& accellerator);
//void init_position_and_momentum(Parameters& parameters, Accellerator& accellerator, ParticleState& particle_state);
