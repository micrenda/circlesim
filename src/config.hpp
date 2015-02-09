#include "type.hpp"
#include <luabind/function.hpp>

void read_config(fs::path& cfg_file, Parameters& parameters, Simulation& simulation, OutputSetting& output, Pulse& laser, Particle& particle, ParticleState& particle_state, Accellerator& accellerator, Node nodes[], lua_State* lua_state);
void init_nodes(Accellerator& accellerator, Node nodes[]);
void init_position_and_momentum(Parameters& parameters, Accellerator& accellerator, ParticleState& particle_state, Node nodes[]);
