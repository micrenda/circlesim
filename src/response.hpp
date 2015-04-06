#include "type.hpp"
double get_attribute(Particle& particle, ParticleStateGlobal& particle_state, Pulse& laser, string object, string attribute);
void   set_attribute(Particle& particle, ParticleStateGlobal& particle_state, Pulse& laser, string object, string attribute, double new_value);
string get_conversion_si_unit(string object, string attribute);
double get_conversion_si_value(string object, string attribute);
void calculateResponseAnalyses(
	ResponseAnalysis& analysis,
	Simulation& simulation,
	Particle& particle,
	ParticleStateGlobal& particle_state_initial, 
	ParticleStateGlobal& particle_state_final,
	Pulse& laser,
	Laboratory& laboratory,
	fs::path output_dir,
	FunctionFieldType function_field,
	FunctionResponseAnalysisCalculated&  on_calculate);
