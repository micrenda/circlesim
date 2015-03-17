#include <LuaState.h>
#include "type.hpp"

void read_config(fs::path& cfg_file, Simulation& simulation, Pulse& laser, Particle& particle, ParticleState& particle_state, Laboratory& laboratory, set<FieldRender*> field_renders, lua::State* lua_state);
