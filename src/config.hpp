#include "type.hpp"

void read_config(
	fs::path& cfg_file,
	Simulation& simulation,
	Pulse& laser,
	Particle& particle,
	ParticleStateGlobal& particle_state,
	Laboratory& laboratory,
	vector<FieldRender>&      field_renders,
	vector<ResponseAnalysis>& response_analyses,
	vector<string>& sources,
	vector<string>& headers);
