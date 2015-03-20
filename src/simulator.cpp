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

int get_near_node_id(ParticleStateGlobal& state, Laboratory& laboratory, double laser_influence_radius)
{
	for (unsigned int r = 0; r < laboratory.nodes.size(); r++)
	{
		Node& node = laboratory.nodes[r];
		
		if (is_in_influence_radius(state, node, laser_influence_radius))
			return r;
	}
	
	return -1;
}



typedef struct gsl_odeiv_custom_params
{
	Pulse* 		laser;
	Particle*	particle;
	lua::State*	lua_state;
} gsl_odeiv_custom_params;

      
void calculate_fields(double pos_t, double pos_x, double pos_y, double pos_z, const Pulse& laser, Field& field, lua::State* lua_state)
{
	//TODO: These values must be cached because does not change between the execution of calculate fiels (or neither in all simulation)
	
  
	double param_duration		=	laser.duration	* AU_TIME;
	double param_time			=	pos_t/C0		* AU_TIME;
	double param_x				=	pos_x 			* AU_LENGTH;
	double param_y				=	pos_y 			* AU_LENGTH;
	double param_z				=	pos_z 			* AU_LENGTH;
  
  
	double e1, e2, e3, b1, b2, b3;
	
	#pragma omp critical (lua_fields)
	{
		// Unfortunately LuaState is not thread safe, so we must not parallel execute this section of code
		lua::tie(e1, e2, e3, b1, b2, b3) = (*lua_state)["func_fields"](param_duration, param_time, param_x, param_y, param_z);
	}
	
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
	Pulse 	 	laser 	  		= *((gsl_odeiv_custom_params*)params)->laser;
	Particle 	particle 		= *((gsl_odeiv_custom_params*)params)->particle;
	lua::State*	lua_state	    =  ((gsl_odeiv_custom_params*)params)->lua_state;

	double pos_t = C0*t;
	double pos_x = y[0];
	double pos_y = y[1];
	double pos_z = y[2];
	double mom_x = y[3];
	double mom_y = y[4];
	double mom_z = y[5];
	
	Field field;
	calculate_fields(pos_t, pos_x, pos_y, pos_z, laser, field, lua_state);
	
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
	Particle particle = *((gsl_odeiv_custom_params*)params)->particle;

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


void simulate_laser(
	Simulation& simulation, 
	Pulse& laser,
	Node& node,
	Particle& particle,
	ParticleStateLocal& state,
	long double& time_current,
	unsigned int interaction,
	lua::State* lua_state)
{
	// Trasforming global coordinates to local coordinates
	

	gsl_odeiv_custom_params params;
	params.laser 	= &laser;
	params.particle	= &particle;
	params.lua_state = lua_state;

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
	
	long double time_start = time_current;
	
	// local time: time ranging in interval ±laser.duration/2
	
	double local_time_start 	= -laser.duration/2;
	double local_time_end   	= +laser.duration/2;
	double local_time_current	=  local_time_start;
	
	while(local_time_current < local_time_end)
	{
		
		double local_time_limit		= local_time_current + simulation.time_resolution_laser;
		double local_time_step		= simulation.time_resolution_laser / 100;
		
		while (local_time_current < local_time_limit)
		{
			gsl_odeiv_evolve_apply(evolve, control, steps, &system, &local_time_current, local_time_limit, &local_time_step, y);
		}
		
		// Update the states
		time_current = time_start + (local_time_current - local_time_start) ;
		
		
		state.position_x = y[0];
		state.position_y = y[1];
		state.position_z = y[2];
		state.momentum_x = y[3];
		state.momentum_y = y[4];
		state.momentum_z = y[5];
		
		// Print the particle state (global position)
		write_particle(stream_particle, time_current, state);
		
		Field field;
		// Print the particle interaction (local position, fields, etc)
		calculate_fields(time_current*C0, y[0], y[1], y[2], laser, field, lua_state);
		write_interaction(stream_interaction, time_current, y[0], y[1], y[2], y[3], y[4], y[5], field);

		// Checking if we are outside the laser influence radius.
		if (is_in_influence_radius(state, node, simulation.laser_influence_radius))
			break;
		
	}



	gsl_odeiv_evolve_free(evolve);
	gsl_odeiv_control_free(control);
	gsl_odeiv_step_free(steps);
}


void simulate_free(Simulation& simulation, Laboratory& laboratory, Particle& particle, ParticleStateGlobal& state, long double& time_current_p, lua::State* lua_state, ofstream& stream_particle)
{
	
	
	double time_current = time_current_p;
	
	// Trasforming global coordinates to local coordinates
	// We use the starting position of the particle as origin for the new reference system
	// We do this to have smaller values and be able to handle them with double type.
	// When updating the 'state' variable we must convert back to the global reference system.
	
	long double origin_x = state.position_x;
	long double origin_y = state.position_y;
	long double origin_z = state.position_z;
	
	double local_pos_x = state.position_x - origin_x; // = 0
	double local_pos_y = state.position_y - origin_y; // = 0
	double local_pos_z = state.position_z - origin_z; // = 0
	
	double local_mom_x = state.momentum_x;
	double local_mom_y = state.momentum_y;
	double local_mom_z = state.momentum_z;
	
	gsl_odeiv_custom_params params;
	params.laser 	 = NULL;
	params.particle	 = &particle;
	params.lua_state = lua_state;

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


	
	while(time_current < simulation.duration)
	{
		double local_time_limit		= time_current + simulation.time_resolution_free;
		double local_time_step		= simulation.time_resolution_free / 100;
		
		while (time_current < local_time_limit)
		{
			gsl_odeiv_evolve_apply(evolve, control, steps, &system, &time_current, local_time_limit, &local_time_step, y);
		}
		
		// Update the state
		state.position_x = origin_x + local_pos_x;
		state.position_y = origin_y + local_pos_y;
		state.position_z = origin_z + local_pos_z;
		state.momentum_x = local_mom_x;
		state.momentum_y = local_mom_y;
		state.momentum_z = local_mom_z;
		
		// Print the state
		write_particle(stream_particle, time_current, state);
		
		// Checking if we are in a range of a laser.
		if (get_near_node_id(state, laboratory, simulation.laser_influence_radius) > 0)
			break;
		
	}

	gsl_odeiv_evolve_free(evolve);
	gsl_odeiv_control_free(control);
	gsl_odeiv_step_free(steps);
	
	
	time_current_p = time_current;

}

void update_limits(FieldRenderResultLimit& limit, double value)
{
	if (value < limit.value_min) limit.value_min = value;
	if (value > limit.value_max) limit.value_max = value;
	double value_abs = abs(value);
	if (value_abs < limit.value_min_abs) limit.value_min_abs = value_abs;
	if (value_abs > limit.value_max_abs) limit.value_max_abs = value_abs;
}


void calculate_field_map(FieldRenderResult& field_render_result, FieldRender& field_render, unsigned int interaction, int node,  Pulse& laser, lua::State* lua_state, fs::path output_dir)
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
	
	
	string wrapper  = "";
	wrapper += "function field(t, x, y, z)\n";
	wrapper += (bo::format("   D=%.16E\n") % (laser.duration * AU_TIME)).str();             
	wrapper += "   value_e_x, value_e_y, value_e_z, value_b_x, value_b_y, value_b_z = func_fields(D, t, x, y, z)\n";
	wrapper += "   return { e_x = value_e_x, e_y = value_e_y, e_z = value_e_z, b_x = value_b_x, b_y = value_b_y, b_z = value_b_z }\n";
	wrapper += "end";
	
	
	try
	{
		lua_state->doString(wrapper);
	}
	catch (lua::LoadError& e)
	{
		printf("Error while parsing 'field': %s\n", e.what());
		printf("--------------------------------------------------------\n");
		printf("%s\n", wrapper.c_str());
		printf("--------------------------------------------------------\n");
		
		exit(-4);
	}

	unsigned int nt = (end_t - start_t) / field_render.time_resolution;
	unsigned int ni = field_render.space_size_x / field_render.space_resolution;
	unsigned int nj = field_render.space_size_y / field_render.space_resolution;
	unsigned int nk = field_render.space_size_z / field_render.space_resolution;
	
	field_render_result.nt = nt;
	
	double****& space = field_render_result.values;
	
	for (unsigned short c = 0; c < field_render.count; c++)
	{
		FieldRenderResultLimit render_limit;
		render_limit.value_min = +INFINITY;
		render_limit.value_max = -INFINITY;
		render_limit.value_min_abs = +INFINITY;
		render_limit.value_max_abs = 0;
		
		field_render_result.limits.push_back(render_limit);
	}

	space = new double***[nt];
	
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
				space[t] = new double**[ni];
				
				double time = start_t + t * field_render.time_resolution;
				
				#pragma omp parallel for shared(space, lua_state)
				for (unsigned int i = 0; i < ni; i++)
				{
					double x = -field_render.space_size_x/2 + field_render.space_resolution * i;
					space[t][i] = new double*[nj];
					for (unsigned int j = 0; j < nj; j++)
					{
						double y = -field_render.space_size_y/2 + field_render.space_resolution * j;
						space[t][i][j] = new double[field_render.count];
						
						double v0,v1,v2,v3,v4,v5,v6,v7;
						
						// Unfortunately LuaState is not thread safe, so we must not parallel execute this section of code
						#pragma omp critical (lua_field_render)
						{
						lua::tie(v0,v1,v2,v3,v4,v5,v6,v7) = (*lua_state)[field_render.func_formula_name.c_str()](laser.duration, time * AU_TIME, x * AU_LENGTH, y * AU_LENGTH, field_render.axis_cut * AU_LENGTH);
						}
						
						for (unsigned short c = 0; c < field_render.count; c++)
						{
							switch(c)
							{
								case 0:
									space[t][i][j][c] = v0;
									update_limits(field_render_result.limits[c], v0);
								break;
								
								case 1:
									space[t][i][j][c] = v1;
									update_limits(field_render_result.limits[c], v1);
								break;
								
								case 2:
									space[t][i][j][c] = v2;
									update_limits(field_render_result.limits[c], v2);
								break;
								
								case 3:
									space[t][i][j][c] = v3;
									update_limits(field_render_result.limits[c], v3);
								break;
								
								case 4:
									space[t][i][j][c] = v4;
									update_limits(field_render_result.limits[c], v4);
								break;
								
								case 5:
									space[t][i][j][c] = v5;
									update_limits(field_render_result.limits[c], v5);
								break;
								
								case 6:
									space[t][i][j][c] = v6;
									update_limits(field_render_result.limits[c], v6);
								break;
								
								case 7:
									space[t][i][j][c] = v7;
									update_limits(field_render_result.limits[c], v7);
								break;
							}
						}
					}
				}
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
				space[t] = new double**[ni];
				
				double time = start_t + t * field_render.time_resolution;
				
				#pragma omp parallel for shared(space, lua_state)
				for (unsigned int i = 0; i < ni; i++)
				{
					double x = -field_render.space_size_x/2 + field_render.space_resolution * i;
					space[t][i] = new double*[nk];
					for (unsigned int k = 0; k < nk; k++)
					{
						double z = -field_render.space_size_z/2 + field_render.space_resolution * k;
						space[t][i][k] = new double[field_render.count];
						
						double v0,v1,v2,v3,v4,v5,v6,v7;
						
						// Unfortunately LuaState is not thread safe, so we must not parallel execute this section of code
						#pragma omp critical (lua_field_render)
						{
						lua::tie(v0,v1,v2,v3,v4,v5,v6,v7) = (*lua_state)[field_render.func_formula_name.c_str()](laser.duration, time * AU_TIME, x * AU_LENGTH, field_render.axis_cut * AU_LENGTH, z * AU_LENGTH);
						}
						
						for (unsigned short c = 0; c < field_render.count; c++)
						{
							switch(c)
							{
								case 0:
									space[t][i][k][c] = v0;
									update_limits(field_render_result.limits[c], v0);
								break;
								
								case 1:
									space[t][i][k][c] = v1;
									update_limits(field_render_result.limits[c], v1);
								break;
								
								case 2:
									space[t][i][k][c] = v2;
									update_limits(field_render_result.limits[c], v2);
								break;
								
								case 3:
									space[t][i][k][c] = v3;
									update_limits(field_render_result.limits[c], v3);
								break;
								
								case 4:
									space[t][i][k][c] = v4;
									update_limits(field_render_result.limits[c], v4);
								break;
								
								case 5:
									space[t][i][k][c] = v5;
									update_limits(field_render_result.limits[c], v5);
								break;
								
								case 6:
									space[t][i][k][c] = v6;
									update_limits(field_render_result.limits[c], v6);
								break;
								
								case 7:
									space[t][i][k][c] = v7;
									update_limits(field_render_result.limits[c], v7);
								break;
							}
						}
					}
				}
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
				space[t] = new double**[nj];
				
				double time = start_t + t * field_render.time_resolution;
				
				#pragma omp parallel for shared(space, lua_state)
				for (unsigned int j = 0; j < nj; j++)
				{
					double y = -field_render.space_size_y/2 + field_render.space_resolution * j;
					space[t][j] = new double*[nk];
					for (unsigned int k = 0; k < nk; k++)
					{
						double z = -field_render.space_size_z/2 + field_render.space_resolution * k;
						space[t][j][k] = new double[field_render.count];
						
						double v0,v1,v2,v3,v4,v5,v6,v7;
						
						// Unfortunately LuaState is not thread safe, so we must not parallel execute this section of code
						#pragma omp critical (lua_field_render)
						{
						lua::tie(v0,v1,v2,v3,v4,v5,v6,v7) = (*lua_state)[field_render.func_formula_name.c_str()](laser.duration, time * AU_TIME, field_render.axis_cut * AU_LENGTH, y * AU_LENGTH, z * AU_LENGTH);
						}
						
						for (unsigned short c = 0; c < field_render.count; c++)
						{
							switch(c)
							{
								case 0:
									space[t][j][k][c] = v0;
									update_limits(field_render_result.limits[c], v0);
								break;
								
								case 1:
									space[t][j][k][c] = v1;
									update_limits(field_render_result.limits[c], v1);
								break;
								
								case 2:
									space[t][j][k][c] = v2;
									update_limits(field_render_result.limits[c], v2);
								break;
								
								case 3:
									space[t][j][k][c] = v3;
									update_limits(field_render_result.limits[c], v3);
								break;
								
								case 4:
									space[t][j][k][c] = v4;
									update_limits(field_render_result.limits[c], v4);
								break;
								
								case 5:
									space[t][j][k][c] = v5;
									update_limits(field_render_result.limits[c], v5);
								break;
								
								case 6:
									space[t][j][k][c] = v6;
									update_limits(field_render_result.limits[c], v6);
								break;
								
								case 7:
									space[t][j][k][c] = v7;
									update_limits(field_render_result.limits[c], v7);
								break;
							}
						}
					}
				}
			}
		break;
		
	}
	
	// Writing space array into a csv file
	export_field_render(field_render_result, output_dir);
	
	// Creating files needed to create the video
	plot_field_render(field_render_result, output_dir);

	// Freeing the allocated memory
	for (unsigned int s1 = 0; s1 < field_render_result.nt; s1++)
	{
		for (unsigned int s2 = 0; s2 < field_render_result.na; s2++)
		{
			for (unsigned int s3 = 0; s3 < field_render_result.nb; s3++)
			{
				delete space[s1][s2][s3];
			}
			delete space[s1][s2];
		}
		delete space[s1];
	}
	delete space;
}	

void simulate (
	Simulation& simulation,
	Pulse& laser,
	Particle& particle,
	ParticleStateGlobal& particle_state,
	Laboratory& laboratory,
	
	function_node on_node_enter,
	function_node on_node_time,
	function_node on_node_exit,
	
	function_free on_free_enter,
	function_free on_free_time,
	function_free on_free_exit,
	
	lua::State* lua_state)
{
	
	//printf("Simulation duration: %f s\n", simulation.duration * AU_TIME);
	
	
	RangeMode 	  current_range = FREE;
	unsigned int  current_interaction = 0;
	int 		  current_node = -1;
	
	//fs::path int_output_dir;
	
	//ofstream stream_particle;
	//ofstream stream_interaction;
	
	//stream_particle.open(get_filename_particle(output_dir));
	//setup_particle(stream_particle);
	
	long double time_current = 0;
	while (time_current < simulation.duration)
	{
		printf("\rSimulating: %3.4f%%", (double)time_current / simulation.duration * 100);
		fflush(stdout);
		
		// Identifing if our particle is inside the laser action range or outside.
		// If outside we use the free motion laws, if inside we calculate the integration between the laser and the particle	
		int new_node = get_near_node_id(particle_state, laboratory, simulation.laser_influence_radius);
		
		if (current_range == LASER && new_node < 0 )
		{
			stream_interaction.close();
			
			
			on_node_exit(simulation, laser, particle, particle_state, laboratory, current_node, long double time_global, double time_local);
			
			
			
			on_free_enter(simulation, laser, particle, particle_state, Laboratory& laboratory,             long double time_global);
			// plot_interaction_files	(int_output_dir);
			
			//for (FieldRender render: field_renders)
			//{
			//	if (render.enabled)
			//	{
					//calculate_field_map (render,  current_interaction, current_node, laser, lua_state, int_output_dir);
			//	}
			//}
			
			current_range = FREE;
			current_node  = -1;
			current_interaction++;
		}
		else if (current_range == FREE && new_node != current_node)
		{
			current_range = LASER;
			current_node = new_node; 
			
			int_output_dir = output_dir / fs::path((bo::format("i%un%u") % current_interaction % current_node).str());
			fs::create_directories(int_output_dir);
			
			stream_interaction.open   (get_filename_interaction		(int_output_dir));
			
			setup_interaction(stream_interaction);
		}
		
		
		if (current_range == LASER)
		{
			Node& node = laboratory.nodes[current_node];
			simulate_laser(
				simulation,
				laser,
				node,
				particle,
				particle_state,
				time_current,
				current_interaction,
				lua_state);	
		}
		else
		{
			simulate_free(
				simulation,
				laboratory,
				particle,
				particle_state,
				time_current,
				lua_state);
		}
	}
	
	printf("\n");
}



	

