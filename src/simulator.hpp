#include <LuaState.h>
#include "type.hpp"

void simulate (Simulation& simulation, set<FieldRender*>& field_renders, Pulse& laser, Particle& particle, ParticleState& particle_state, Laboratory& laboratory, lua::State* lua_state, fs::path& output_dir);
