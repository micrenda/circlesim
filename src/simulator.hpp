#include <LuaState.h>
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
	
	lua::State* lua_state);

void simulate (
	Simulation& simulation,
	Pulse& laser,
	Particle& particle,
	ParticleStateGlobal& particle_state_global,
	Laboratory& laboratory,
	vector<SimluationResultFreeSummary>& summaries_free,
	vector<SimluationResultNodeSummary>& summaries_node,
	lua::State* lua_state);
	
void calculate_field_map(FieldRenderResult& field_render_result, FieldRender& field_render, unsigned int interaction, int node,  Pulse& laser, lua::State* lua_state, fs::path output_dir);
