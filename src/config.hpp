#include <LuaState.h>
#include "type.hpp"

void read_config(
	fs::path& cfg_file,
	Simulation& simulation,
	Pulse& laser,
	Particle& particle,
	ParticleStateGlobal& particle_state,
	Laboratory& laboratory,
	set<FieldRender>&      field_renders,
	set<ResponseAnalysis>& response_analyses,
	lua::State* lua_state);
