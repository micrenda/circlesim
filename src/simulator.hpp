#include <LuaState.h>
#include "type.hpp"

void simulate (Simulation& simulation, OutputSetting& output_setting, Pulse& laser, Particle& particle, ParticleState& particle_state, Accellerator& accellerator, lua::State* lua_state, fs::path& output_dir);
