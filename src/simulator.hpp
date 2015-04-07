#include "type.hpp"

void simulate (
	Simulation& simulation,
	Pulse& laser,
	Particle& particle,
	ParticleStateGlobal& particle_state_global,
	Laboratory& laboratory,
	
	FunctionNodeEnter&        on_node_enter,
	FunctionNodeTimeProgress& on_node_time_progress,
	FunctionNodeExit&         on_node_exit,
	
	FunctionFreeEnter&        on_free_enter,
	FunctionFreeTimeProgress& on_free_time_progress,
	FunctionFreeExit&         on_free_exit,
	
	vector<SimluationResultFreeSummary>& summariesFree,
	vector<SimluationResultNodeSummary>& summariesNode,
	
	FunctionFieldType function_field);

void simulate (
	Simulation& simulation,
	Pulse& laser,
	Particle& particle,
	ParticleStateGlobal& particle_state_global,
	Laboratory& laboratory,
	vector<SimluationResultFreeSummary>& summaries_free,
	vector<SimluationResultNodeSummary>& summaries_node,
	FunctionFieldType function_field);
	

void calculate_fields(double pos_t, double pos_x, double pos_y, double pos_z, const Pulse& laser, Field& field, FunctionFieldType function_field);
