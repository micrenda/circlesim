#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>
#include "simulator.hpp"
#include "type.hpp"
#include "util.hpp"
#include "output.hpp"
#include "plot.hpp"




bool is_in_influence_radius(ParticleStateGlobal& state, Node& node, double laser_influence_radius)
{
	return vector_module(
			state.position_x - node.position_x,
			state.position_y - node.position_y,
			state.position_z - node.position_z) <= laser_influence_radius;
}

bool is_in_influence_radius(ParticleStateLocal& state, double laser_influence_radius)
{
	return vector_module(state.position_x, state.position_y, state.position_z) <= laser_influence_radius;
}

int get_near_node_id(ParticleStateGlobal& state, Laboratory& laboratory, double laser_influence_radius)
{
	for (Node& node: laboratory.nodes)
	{
		if (is_in_influence_radius(state, node, laser_influence_radius))
			return node.id;
	}
	
	return -1;
}



typedef struct gsl_odeiv_custom_params
{
	Pulse* 					laser;
	Particle*				particle;
	vector<lua::State*>*	lua_states;

} gsl_odeiv_custom_params;

      
void calculate_fields(double pos_t, double pos_x, double pos_y, double pos_z, const Pulse& laser, Field& field, vector<lua::State*>& lua_states)
{
	//TODO: These values must be cached because does not change between the execution of calculate fiels (or neither in all simulation)
	
	double param_time			=	pos_t/C0		* AU_TIME;
	double param_x				=	pos_x 			* AU_LENGTH;
	double param_y				=	pos_y 			* AU_LENGTH;
	double param_z				=	pos_z 			* AU_LENGTH;
  
  
	double e1, e2, e3, b1, b2, b3;
	

	lua::State* local_lua_state = lua_states[omp_get_thread_num()];	
	lua::String f_name = lua::String((get_thread_prefix(omp_get_thread_num()) + "func_fields").c_str());
	lua::tie(e1, e2, e3, b1, b2, b3) = (*local_lua_state)[f_name](param_time, param_x, param_y, param_z);

	
	field.e_x = e1 / AU_ELECTRIC_FIELD; 
	field.e_y = e2 / AU_ELECTRIC_FIELD;
	field.e_z = e3 / AU_ELECTRIC_FIELD;
  
    field.b_x = b1 / AU_MAGNETIC_FIELD;
	field.b_y = b2 / AU_MAGNETIC_FIELD;
	field.b_z = b3 / AU_MAGNETIC_FIELD; 

}
      
      
int gsl_odeiv_jac (double t, const double y[], double *dfdy, double dfdt[], void *params)
{
	cout << "jacobian used\n";
	return GSL_SUCCESS;
}

int gsl_odeiv_func_laser(double t, const double y[], double f[], void *params)
{
	Pulse& 	 	laser 	  				= *((gsl_odeiv_custom_params*)params)->laser;
	Particle& 	particle 				= *((gsl_odeiv_custom_params*)params)->particle;
	vector<lua::State*>& lua_states     = *((gsl_odeiv_custom_params*)params)->lua_states;

	double pos_t = C0*t;
	double pos_x = y[0];
	double pos_y = y[1];
	double pos_z = y[2];
	double mom_x = y[3];
	double mom_y = y[4];
	double mom_z = y[5];
	
	Field field;
	calculate_fields(pos_t, pos_x, pos_y, pos_z, laser, field, lua_states);
	
	double e_x = field.e_x;
	double e_y = field.e_x;
	double e_z = field.e_x;
	
	double b_x = field.e_x;
	double b_y = field.e_y;
	double b_z = field.e_z;
	
	// In out formulas we use B instead of H and we use the relation B = H/c₀
	// Formulas (3),(4),(5)
	
	// fac =  1/√(1 + p/(c₀m)²) = c₀/√(c₀²+p²)
	double fac = C0 / sqrt(C0 * C0 + mom_x * mom_x + mom_y * mom_y + mom_z * mom_z);
	
	f[0] = fac * mom_x / particle.rest_mass;
	f[1] = fac * mom_y / particle.rest_mass;
	f[2] = fac * mom_z / particle.rest_mass;
	f[3] = - particle.charge * e_x - particle.charge / particle.rest_mass * fac * (mom_y * b_z - mom_z * b_y);
	f[4] = - particle.charge * e_y + particle.charge / particle.rest_mass * fac * (mom_z * b_x - mom_x * b_z);
	f[5] = - particle.charge * e_z - particle.charge / particle.rest_mass * fac * (mom_x * b_y - mom_y * b_x);

	return GSL_SUCCESS;
}

int gsl_odeiv_func_free(double t, const double y[], double f[], void *params)
{
	Particle& particle = *((gsl_odeiv_custom_params*)params)->particle;

	// double pos_t = C0*t;
	// double pos_x = y[0];
	// double pos_y = y[1];
	// double pos_z = y[2];
	double mom_x = y[3];
	double mom_y = y[4];
	double mom_z = y[5];
	
	double fac=C0/sqrt(C0 * C0 + mom_x * mom_x + mom_y * mom_y + mom_z * mom_z);
	
	f[0] = fac * mom_x / particle.rest_mass;
	f[1] = fac * mom_y / particle.rest_mass;
	f[2] = fac * mom_z / particle.rest_mass;
	f[3] = 0;
	f[4] = 0;
	f[5] = 0;

	return GSL_SUCCESS;
}


void simulate_node(
	Simulation& simulation, 
	Pulse& laser,
	Node& node,
	Particle& particle,
	ParticleStateLocal& state,
	double&  local_time_current,
	unsigned int interaction,
	FunctionNodeTimeProgress& on_node_time_progress,
	SimluationResultNodeSummary& summary,
	vector<lua::State*>& lua_states)
{
	// Trasforming global coordinates to local coordinates
	
	
	summary.time_enter = local_time_current;

	gsl_odeiv_custom_params params;
	params.laser 	  = &laser;
	params.particle	  = &particle;
	params.lua_states = &lua_states;

	const gsl_odeiv_step_type* 	step_type 	= gsl_odeiv_step_rk8pd;
	gsl_odeiv_step* 			steps 		= gsl_odeiv_step_alloc(step_type, 6);
	gsl_odeiv_control* 			control		= gsl_odeiv_control_y_new (simulation.error_abs, simulation.error_rel);
	gsl_odeiv_evolve* 			evolve		= gsl_odeiv_evolve_alloc(6);
	gsl_odeiv_system 			system 		= {gsl_odeiv_func_laser, gsl_odeiv_jac, 6, &params};

	
	double y[6];
	y[0] = state.position_x;
	y[1] = state.position_y;
	y[2] = state.position_z;
	y[3] = state.momentum_x;
	y[4] = state.momentum_y;
	y[5] = state.momentum_z;
	
	
	while(true)
	{
		
		double local_time_limit		= local_time_current + simulation.time_resolution_laser;
		double local_time_step		= simulation.time_resolution_laser / 100;
		
		while (local_time_current < local_time_limit)
		{
			gsl_odeiv_evolve_apply(evolve, control, steps, &system, &local_time_current, local_time_limit, &local_time_step, y);
		}
		
		
		state.position_x = y[0];
		state.position_y = y[1];
		state.position_z = y[2];
		state.momentum_x = y[3];
		state.momentum_y = y[4];
		state.momentum_z = y[5];
		
		Field field;
		// Print the particle interaction (local position, fields, etc)
		calculate_fields(local_time_current*C0, y[0], y[1], y[2], laser, field, lua_states);
		
		if (on_node_time_progress != NULL) on_node_time_progress(simulation, laser, particle, state, interaction, node, local_time_current, field);
		//TODO:
		//write_particle(stream_particle, time_current, state);
		//write_interaction(stream_interaction, time_current, y[0], y[1], y[2], y[3], y[4], y[5], field);
		
		SimluationResultLaserItem result;
		result.time		= local_time_current;
		result.state	= state;
		result.field	= field;
		summary.items.push_back(result);
		
		
		// Checking if we are outside the laser influence radius.
		if (!is_in_influence_radius(state, simulation.laser_influence_radius))
			break;
		
	}



	gsl_odeiv_evolve_free(evolve);
	gsl_odeiv_control_free(control);
	gsl_odeiv_step_free(steps);
	
	summary.time_exit = local_time_current;
}


void simulate_free(Simulation& simulation, Laboratory& laboratory, Particle& particle, ParticleStateGlobal& state, long double& global_time_current, FunctionFreeTimeProgress& on_free_time_progress, SimluationResultFreeSummary& summary, vector<lua::State*>& lua_states)
{
	
	summary.time_enter = global_time_current;
	
	// Trasforming global coordinates to local coordinates
	// We use the starting position of the particle as origin for the new reference system
	// We do this to have smaller values and be able to handle them with double type.
	// When updating the 'state' variable we must convert back to the global reference system.
	
	long double origin_x = state.position_x;
	long double origin_y = state.position_y;
	long double origin_z = state.position_z;
	long double origin_t = global_time_current;
	
	double local_pos_x = state.position_x - origin_x; // = 0
	double local_pos_y = state.position_y - origin_y; // = 0
	double local_pos_z = state.position_z - origin_z; // = 0
	double local_t     = global_time_current - origin_t; // = 0
	
	double local_mom_x = state.momentum_x;
	double local_mom_y = state.momentum_y;
	double local_mom_z = state.momentum_z;
	
	gsl_odeiv_custom_params params;
	params.laser 	 = NULL;
	params.particle	 = &particle;
	params.lua_states = &lua_states;

	const gsl_odeiv_step_type* 	step_type 	= gsl_odeiv_step_rk8pd;
	gsl_odeiv_step* 			steps 		= gsl_odeiv_step_alloc(step_type, 6);
	gsl_odeiv_control* 			control		= gsl_odeiv_control_y_new (simulation.error_abs, simulation.error_rel);
	gsl_odeiv_evolve* 			evolve		= gsl_odeiv_evolve_alloc(6);
	gsl_odeiv_system 			system 		= {gsl_odeiv_func_free, gsl_odeiv_jac, 6, &params};

	
	double y[6];
	y[0] = local_pos_x;
	y[1] = local_pos_y;
	y[2] = local_pos_z;
	y[3] = local_mom_x;
	y[4] = local_mom_y;
	y[5] = local_mom_z;
	
	SimluationResultFreeItem head_result;
	head_result.time		= global_time_current;
	head_result.state	= state;
	summary.items.push_back(head_result);


	
	while(global_time_current < simulation.duration)
	{
		double local_time_limit		= local_t + simulation.time_resolution_free;
		double local_time_step		= simulation.time_resolution_free / 100;
		
		while (local_t < local_time_limit)
		{
			gsl_odeiv_evolve_apply(evolve, control, steps, &system, &local_t, local_time_limit, &local_time_step, y);
		}
		
		// Update the state
		state.position_x = origin_x + y[0];
		state.position_y = origin_y + y[1];
		state.position_z = origin_z + y[2];
		state.momentum_x = y[3];
		state.momentum_y = y[4];
		state.momentum_z = y[5];
		
		global_time_current = origin_t + local_t;
		
		if (on_free_time_progress != NULL) on_free_time_progress(simulation, particle, state, laboratory, global_time_current);
		
		SimluationResultFreeItem result;
		result.time		= global_time_current;
		result.state	= state;
		summary.items.push_back(result);
		
		// Checking if we are in a range of a laser.
		if (get_near_node_id(state, laboratory, simulation.laser_influence_radius) >= 0)
			break;
		
	}

	gsl_odeiv_evolve_free(evolve);
	gsl_odeiv_control_free(control);
	gsl_odeiv_step_free(steps);
	
	
	summary.time_exit = global_time_current;

}

void update_limits(FieldRenderResultLimit& limit, double value)
{
	if (value < limit.value_min) limit.value_min = value;
	if (value > limit.value_max) limit.value_max = value;
	double value_abs = abs(value);
	if (value_abs < limit.value_min_abs) limit.value_min_abs = value_abs;
	if (value_abs > limit.value_max_abs) limit.value_max_abs = value_abs;
}


void calculate_field_map(FieldRenderResult& field_render_result, FieldRender& field_render, unsigned int interaction, int node,  Pulse& laser, vector<lua::State*>& lua_states, fs::path output_dir)
{
	
	// Initializing field_render_result
	
	field_render_result.node 		= node;
	field_render_result.interaction	= interaction;
	
	field_render_result.render = field_render;
	

	
	//lua_state->set("field_c_wrapper", [laser, lua_state] (double time, double pos_x, double pos_y, double pos_z) -> tuple<double, double, double, double, double, double>
	//{
		//Field field_value;
		//calculate_fields(
			//time * C0 / AU_LENGTH,
			//pos_x / AU_LENGTH,
			//pos_y / AU_LENGTH,
			//pos_z / AU_LENGTH,
			//laser,
			//field_value,
			//lua_state);
		
		//field_value.e_x *= AU_ELECTRIC_FIELD;
		//field_value.e_y *= AU_ELECTRIC_FIELD;
		//field_value.e_z *= AU_ELECTRIC_FIELD;
		
		//field_value.b_x *= AU_MAGNETIC_FIELD;
		//field_value.b_y *= AU_MAGNETIC_FIELD;
		//field_value.b_z *= AU_MAGNETIC_FIELD;
		
		//tuple<double, double, double, double, double, double> values = make_tuple
		//(	field_value.e_x,
			//field_value.e_y,
			//field_value.e_z,
			//field_value.b_x,
			//field_value.b_y,
			//field_value.b_z);
			
		
		//return values;
	//});
	
	//string wrapper = "
	//function field(t, x, y, z)                    
		//value_e_x, value_e_y, value_e_z, value_b_x, value_b_y, value_b_z = field_c_wrapper(t, x, y, z)  
		//return { e_x = value_e_x, e_y = value_e_y, e_z = value_e_z, b_x = value_b_x, b_y = value_b_y, b_z = value_b_z }          
	//end";

	
	field_render_result.time_start = field_render.time_start;
	field_render_result.time_end   = field_render.time_end;
	
	double& start_t = field_render_result.time_start;
	double& end_t   = field_render_result.time_end;
	

	unsigned int nt = (end_t - start_t) / field_render.time_resolution;
	unsigned int ni = field_render.space_size_x / field_render.space_resolution;
	unsigned int nj = field_render.space_size_y / field_render.space_resolution;
	unsigned int nk = field_render.space_size_z / field_render.space_resolution;
	
	field_render_result.nt = nt;
	

	for (unsigned short c = 0; c < field_render.count; c++)
	{
		FieldRenderResultLimit render_limit;
		render_limit.value_min = +INFINITY;
		render_limit.value_max = -INFINITY;
		render_limit.value_min_abs = +INFINITY;
		render_limit.value_max_abs = 0;
		
		field_render_result.limits.push_back(render_limit);
	}

	
	
	// Creating space arrays with the field for the entire duration
	switch(field_render.plane)
	{
		case XY:
			field_render_result.axis1_label = "x";
			field_render_result.axis2_label = "y";
			field_render_result.na = ni;
			field_render_result.nb = nj;
			field_render_result.length_a = field_render.space_size_x;
			field_render_result.length_b = field_render.space_size_y;
		
			for (unsigned int t = 0; t < nt; t++)
			{
				printf("\rRendering %s: %.3f%%", field_render.id.c_str(), 100.d * (t+1) / nt);
				fflush(stdout);
				
				
				double time = start_t + t * field_render.time_resolution;
				
				FieldRenderData data;
				data.t = t;
				data.values = new double**[ni];
				
				#pragma omp parallel for shared(data, lua_states)
				for (unsigned int i = 0; i < ni; i++)
				{
					double x = -field_render.space_size_x/2 + field_render.space_resolution * i;
					data.values[i] = new double*[nj];
					for (unsigned int j = 0; j < nj; j++)
					{
						double y = -field_render.space_size_y/2 + field_render.space_resolution * j;
						data.values[i][j] = new double[field_render.count];
						
						double v0,v1,v2,v3,v4,v5,v6,v7;
						
						lua::State* local_lua_state = lua_states[omp_get_thread_num()];	
						lua::String f_name = lua::String((get_thread_prefix(omp_get_thread_num()) + field_render.func_formula_name).c_str());
						lua::tie(v0,v1,v2,v3,v4,v5,v6,v7) = (*local_lua_state)[f_name](time * AU_TIME, x * AU_LENGTH, y * AU_LENGTH, field_render.axis_cut * AU_LENGTH);
						
						
						for (unsigned short c = 0; c < field_render.count; c++)
						{
							switch(c)
							{
								case 0:
									data.values[i][j][c] = v0;
									update_limits(field_render_result.limits[c], v0);
								break;
								
								case 1:
									data.values[i][j][c] = v1;
									update_limits(field_render_result.limits[c], v1);
								break;
								
								case 2:
									data.values[i][j][c] = v2;
									update_limits(field_render_result.limits[c], v2);
								break;
								
								case 3:
									data.values[i][j][c] = v3;
									update_limits(field_render_result.limits[c], v3);
								break;
								
								case 4:
									data.values[i][j][c] = v4;
									update_limits(field_render_result.limits[c], v4);
								break;
								
								case 5:
									data.values[i][j][c] = v5;
									update_limits(field_render_result.limits[c], v5);
								break;
								
								case 6:
									data.values[i][j][c] = v6;
									update_limits(field_render_result.limits[c], v6);
								break;
								
								case 7:
									data.values[i][j][c] = v7;
									update_limits(field_render_result.limits[c], v7);
								break;
							}
						}
					}
				}
				
				save_field_render_data(field_render_result, data, output_dir);
				
				// Freeing the allocated memory
				for (unsigned int s2 = 0; s2 < field_render_result.na; s2++)
				{
					for (unsigned int s3 = 0; s3 < field_render_result.nb; s3++)
					{
						delete [] data.values[s2][s3];
					}
					delete [] data.values[s2];
				}
				delete []  data.values;
			}
		break;
		
		case XZ:
			field_render_result.axis1_label = "x";
			field_render_result.axis2_label = "z";
			field_render_result.na = ni;
			field_render_result.nb = nk;
			field_render_result.length_a = field_render.space_size_x;
			field_render_result.length_b = field_render.space_size_z;
		
			for (unsigned int t = 0; t < nt; t++)
			{
				printf("\rRendering %s: %.3f%%", field_render.id.c_str(), 100.d * (t+1) / nt);
				fflush(stdout);
				
				double time = start_t + t * field_render.time_resolution;
				
				FieldRenderData data;
				data.t = t;
				data.values = new double**[ni];
				
				#pragma omp parallel for shared(data, lua_states)
				for (unsigned int i = 0; i < ni; i++)
				{
					double x = -field_render.space_size_x/2 + field_render.space_resolution * i;
					data.values[i] = new double*[nk];
					for (unsigned int k = 0; k < nk; k++)
					{
						double z = -field_render.space_size_z/2 + field_render.space_resolution * k;
						data.values[i][k] = new double[field_render.count];
						
						double v0,v1,v2,v3,v4,v5,v6,v7;
						
						lua::State* local_lua_state = lua_states[omp_get_thread_num()];
						lua::String f_name = lua::String((get_thread_prefix(omp_get_thread_num()) + field_render.func_formula_name).c_str());
						lua::tie(v0,v1,v2,v3,v4,v5,v6,v7) = (*local_lua_state)[f_name](time * AU_TIME, x * AU_LENGTH, field_render.axis_cut * AU_LENGTH, z * AU_LENGTH);
						
						
						for (unsigned short c = 0; c < field_render.count; c++)
						{
							switch(c)
							{
								case 0:
									data.values[i][k][c] = v0;
									update_limits(field_render_result.limits[c], v0);
								break;
								
								case 1:
									data.values[i][k][c] = v1;
									update_limits(field_render_result.limits[c], v1);
								break;
								
								case 2:
									data.values[i][k][c] = v2;
									update_limits(field_render_result.limits[c], v2);
								break;
								
								case 3:
									data.values[i][k][c] = v3;
									update_limits(field_render_result.limits[c], v3);
								break;
								
								case 4:
									data.values[i][k][c] = v4;
									update_limits(field_render_result.limits[c], v4);
								break;
								
								case 5:
									data.values[i][k][c] = v5;
									update_limits(field_render_result.limits[c], v5);
								break;
								
								case 6:
									data.values[i][k][c] = v6;
									update_limits(field_render_result.limits[c], v6);
								break;
								
								case 7:
									data.values[i][k][c] = v7;
									update_limits(field_render_result.limits[c], v7);
								break;
							}
						}
					}
				}
				
				save_field_render_data(field_render_result, data, output_dir);
				
				// Freeing the allocated memory
				for (unsigned int s2 = 0; s2 < field_render_result.na; s2++)
				{
					for (unsigned int s3 = 0; s3 < field_render_result.nb; s3++)
					{
						delete [] data.values[s2][s3];
					}
					delete [] data.values[s2];
				}
				delete [] data.values;
			}
		break;
		
		case YZ:
			field_render_result.axis1_label = "y";
			field_render_result.axis2_label = "z";
			field_render_result.na = nj;
			field_render_result.nb = nk;
			field_render_result.length_a = field_render.space_size_y;
			field_render_result.length_b = field_render.space_size_z;
		
			for (unsigned int t = 0; t < nt; t++)
			{
				printf("\rRendering %s: %.3f%%", field_render.id.c_str(), 100.d * (t+1) / nt);
				fflush(stdout);
				
				double time = start_t + t * field_render.time_resolution;
				
				FieldRenderData data;
				data.t = t;
				data.values = new double**[nj];
				
				#pragma omp parallel for shared(data, lua_states)
				for (unsigned int j = 0; j < nj; j++)
				{
					double y = -field_render.space_size_y/2 + field_render.space_resolution * j;
					data.values[j] = new double*[nk];
					for (unsigned int k = 0; k < nk; k++)
					{
						double z = -field_render.space_size_z/2 + field_render.space_resolution * k;
						data.values[j][k] = new double[field_render.count];
						
						double v0,v1,v2,v3,v4,v5,v6,v7;
						
						lua::State* local_lua_state = lua_states[omp_get_thread_num()];
						lua::String f_name = lua::String((get_thread_prefix(omp_get_thread_num()) + field_render.func_formula_name).c_str());
						lua::tie(v0,v1,v2,v3,v4,v5,v6,v7) = (*local_lua_state)[f_name](time * AU_TIME, field_render.axis_cut * AU_LENGTH, y * AU_LENGTH, z * AU_LENGTH);
						
						
						for (unsigned short c = 0; c < field_render.count; c++)
						{
							switch(c)
							{
								case 0:
									data.values[j][k][c] = v0;
									update_limits(field_render_result.limits[c], v0);
								break;
								
								case 1:
									data.values[j][k][c] = v1;
									update_limits(field_render_result.limits[c], v1);
								break;
								
								case 2:
									data.values[j][k][c] = v2;
									update_limits(field_render_result.limits[c], v2);
								break;
								
								case 3:
									data.values[j][k][c] = v3;
									update_limits(field_render_result.limits[c], v3);
								break;
								
								case 4:
									data.values[j][k][c] = v4;
									update_limits(field_render_result.limits[c], v4);
								break;
								
								case 5:
									data.values[j][k][c] = v5;
									update_limits(field_render_result.limits[c], v5);
								break;
								
								case 6:
									data.values[j][k][c] = v6;
									update_limits(field_render_result.limits[c], v6);
								break;
								
								case 7:
									data.values[j][k][c] = v7;
									update_limits(field_render_result.limits[c], v7);
								break;
							}
						}
					}
				}
				save_field_render_data(field_render_result, data, output_dir);
				
				// Freeing the allocated memory
				for (unsigned int s2 = 0; s2 < field_render_result.na; s2++)
				{
					for (unsigned int s3 = 0; s3 < field_render_result.nb; s3++)
					{
						delete [] data.values[s2][s3];
					}
					delete [] data.values[s2];
				}
				delete [] data.values;
			}
		break;
		
	}
	
	// Creating files needed to create the video
	save_field_render_ct2(field_render_result, output_dir);
	save_field_render_sh(field_render_result, output_dir);
}	
	
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
	
	vector<SimluationResultFreeSummary>& summaries_free,
	vector<SimluationResultNodeSummary>& summaries_node,
	
	vector<lua::State*>& lua_states)
{
	
	RangeMode 	  current_range = UNKN;
	unsigned int  current_interaction = 0;
	int 		  current_node = -1;
	
	long double time_current_global = 0;
	double      time_current_local;
	
	ParticleStateLocal particle_state_local;
	
	while (time_current_global < simulation.duration)
	{
		// Identifing if our particle is inside the laser action range or outside.
		// If outside we use the free motion laws, if inside we calculate the integration between the laser and the particle	
		int new_node = get_near_node_id(particle_state_global, laboratory, simulation.laser_influence_radius);
		
		if (current_range != FREE && new_node < 0 )
		{
			Node& node = laboratory.nodes[current_node];
			
			if (current_range == NODE)
			{
				if (on_node_exit != NULL) on_node_exit (simulation, laser, particle, particle_state_local, current_interaction, node, time_current_local);
					state_local_to_global(particle_state_global, particle_state_local, node);
			}
			
			if (on_free_enter != NULL) on_free_enter(simulation, particle, particle_state_global, laboratory,   time_current_global);
			
			current_range = FREE;
			current_node  = -1;
			
			current_interaction++;
			
		}
		else if (current_range != NODE && new_node != current_node)
		{
			if (current_range == FREE && on_free_exit != NULL) on_free_exit(simulation, particle, particle_state_global, laboratory, time_current_global);
			
			current_range = NODE;
			current_node = new_node; 
			Node& node = laboratory.nodes[current_node];
			
			state_global_to_local(particle_state_local, particle_state_global, node);
			if (on_node_enter != NULL) on_node_enter(simulation, laser, particle,  particle_state_local, current_interaction, node, time_current_local);
		}
		
		
		if (current_range == NODE)
		{
			Node& node = laboratory.nodes[current_node];
			
			double before = time_current_local;
			
			SimluationResultNodeSummary summary;
			summary.node = node;
			simulate_node(simulation, laser, node, particle, particle_state_local, time_current_local, current_interaction, on_node_time_progress, summary, lua_states);
			summaries_node.push_back(summary);	
			state_local_to_global(particle_state_global, particle_state_local, node);
			
			time_current_global += time_current_local - before;
		}
		else if (current_range == FREE)
		{
			SimluationResultFreeSummary summary;
			simulate_free(simulation, laboratory, particle, particle_state_global, time_current_global, on_free_time_progress, summary, lua_states);
			summaries_free.push_back(summary);
		}
	}
}



void simulate (
	Simulation& simulation,
	Pulse& laser,
	Particle& particle,
	ParticleStateGlobal& particle_state_global,
	Laboratory& laboratory,
	vector<SimluationResultFreeSummary>& summaries_free,
	vector<SimluationResultNodeSummary>& summaries_node,
	vector<lua::State*>& lua_states)
{
	
	FunctionNodeEnter        on_node_enter			= [&](Simulation& simulation, Pulse& laser, Particle& particle, ParticleStateLocal&  particle_state, unsigned int current_interaction, Node& node, double time_local) mutable {};
	FunctionNodeTimeProgress on_node_time_progress	= [&](Simulation& simulation, Pulse& laser, Particle& particle, ParticleStateLocal&  particle_state, unsigned int current_interaction, Node& node, double time_local, Field& field) mutable {};
	FunctionNodeExit         on_node_exit			= [&](Simulation& simulation, Pulse& laser, Particle& particle, ParticleStateLocal&  particle_state, unsigned int current_interaction, Node& node, double time_local) mutable {};

	FunctionFreeEnter        on_free_enter			= [&](Simulation& simulation, Particle& particle, ParticleStateGlobal& particle_state, Laboratory& laboratory, long double time_global) mutable {};
	FunctionFreeTimeProgress on_free_time_progress	= [&](Simulation& simulation, Particle& particle, ParticleStateGlobal& particle_state, Laboratory& laboratory, long double time_global) mutable {};
	FunctionFreeExit         on_free_exit			= [&](Simulation& simulation, Particle& particle, ParticleStateGlobal& particle_state, Laboratory& laboratory, long double time_global) mutable {};
	
	simulate (
		simulation,
		laser,
		particle,
		particle_state_global,
		laboratory,
		on_node_enter,
		on_node_time_progress,
		on_node_exit,
		on_free_enter,
		on_free_time_progress,
		on_free_exit,
		summaries_free,
		summaries_node,
		lua_states);
}

	

