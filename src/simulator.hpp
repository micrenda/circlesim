#include <LuaState.h>
#include "type.hpp"

void simulate (Simulation& simulation, OutputSetting& output_setting, Pulse& laser, Particle& particle, ParticleState& particle_state, Laboratory& laboratory, lua::State* lua_state, fs::path& output_dir);
