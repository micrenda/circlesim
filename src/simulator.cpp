#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>
#include "simulator.hpp"
#include "type.hpp"
#include "util.hpp"
#include "output.hpp"
#include "plot.hpp"


bool is_in_influence_radius(ParticleState& state, Node& node, double laser_influence_radius)
{
	return vector_module(
			state.position_x - node.position_x,
			state.position_y - node.position_y,
			state.position_z - node.position_z) <= laser_influence_radius;
}

int get_near_node_id(ParticleState& state, int nodes_count, Node nodes[], double laser_influence_radius)
{
	for (int r = 0; r < nodes_count; r++)
	{
		Node& node = nodes[r];
		
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

      
void calculate_fields(double pos_t, double pos_x, double pos_y, double pos_z, Pulse& laser, FieldEB& field, lua::State* lua_state)
{
	//TODO: These values must be cached because does not change between the execution of calculate fiels (or neither in all simulation)
	
  
	double param_duration		=	laser.duration	* AU_TIME;
	double param_time			=	pos_t/C0		* AU_TIME;
	double param_x				=	pos_x 			* AU_LENGTH;
	double param_y				=	pos_y 			* AU_LENGTH;
	double param_z				=	pos_z 			* AU_LENGTH;
  
  
	double e1, e2, e3, b1, b2, b3;
	lua::tie(e1, e2, e3, b1, b2, b3) = (*lua_state)["func_fields"](param_duration, param_time, param_x, param_y, param_z);
	
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
	
	FieldEB field;
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


void simulate_laser(Simulation& simulation, Pulse& laser, Node& node, Particle& particle, ParticleState& state, long double& time_current, unsigned int interaction, lua::State* lua_state)
{
	// Trasforming global coordinates to local coordinates
	
	double local_pos_x;
	double local_pos_y; 
	double local_pos_z; 
	
	double local_mom_x;
	double local_mom_y; 
	double local_mom_z; 
	
	state_global_to_local(state, node, local_pos_x, local_pos_y, local_pos_z, local_mom_x, local_mom_y, local_mom_z);
	
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
	y[0] = local_pos_x;
	y[1] = local_pos_y;
	y[2] = local_pos_z;
	y[3] = local_mom_x;
	y[4] = local_mom_y;
	y[5] = local_mom_z;
	
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
		state_local_to_global(state, node, y[0], y[1], y[2], y[3], y[4], y[5]);
		
		
		// Print the particle state (global position)
		write_particle(time_current, state);
		
		FieldEB field;
		// Print the particle interaction (local position, fields, etc)
		calculate_fields(time_current*C0, y[0], y[1], y[2], laser, field, lua_state);
		write_interaction(time_current, y[0], y[1], y[2], y[3], y[4], y[5], field);
		
		// print the field in several points
		calculate_fields(time_current*C0, 0, 0, 0, laser, field, lua_state);
		write_field(time_current, 0, 0, 0, field);
		
		
		// Checking if we are outside the laser influence radius.
		if (is_in_influence_radius(state, node, simulation.laser_influence_radius))
			break;
		
	}



	gsl_odeiv_evolve_free(evolve);
	gsl_odeiv_control_free(control);
	gsl_odeiv_step_free(steps);
}


void simulate_free(Simulation& simulation, Particle& particle, ParticleState& state, long double& time_current_p, int nodes_count, Node nodes[], lua::State* lua_state)
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
		write_particle(time_current, state);
		
		// Checking if we are in a range of a laser.
		if (get_near_node_id(state, nodes_count, nodes, simulation.laser_influence_radius) > 0)
			break;
		
	}

	gsl_odeiv_evolve_free(evolve);
	gsl_odeiv_control_free(control);
	gsl_odeiv_step_free(steps);
	
	
	time_current_p = time_current;

}

void simulate_field_maps(OutputSetting& output_setting, unsigned int interaction, int node, double start_t, double end_t, double trigger_t, Pulse& laser, lua::State* lua_state, fs::path output_dir)
{
	double dt = output_setting.field_map_resolution_t;
	double dx = output_setting.field_map_resolution_x;
	double dy = output_setting.field_map_resolution_y;
	double dz = output_setting.field_map_resolution_z;
	
	int t_count = round((end_t - start_t) / dt);
	
	
	if (output_setting.field_map_enable_xy)
	{
		#pragma omp parallel for default(shared)
		for (int t = 0; t < t_count; t++)
		{
			double current_t = t * dt;
			open_field_map_xy_files(output_dir, interaction, node, t);
				
			for (double x = -output_setting.field_map_size_x/2; x < output_setting.field_map_size_x/2; x += dx)
			{
				for (double y = -output_setting.field_map_size_y/2; y < output_setting.field_map_size_y/2; y += dy)
				{
					FieldEB field;
					calculate_fields(C0*(current_t - trigger_t), x, y, 0, laser, field, lua_state);
					write_field_maps_xy(current_t, x, y, 0, field);
				}
			}
			close_field_map_xy_files();
		}
	}

	if (output_setting.field_map_enable_xz)
	{
		#pragma omp parallel for default(shared)
		for (int t = 0; t < t_count; t++)
		{	
			double current_t = t * dt;
			open_field_map_xz_files(output_dir, interaction, node, t);
			
			for (double x = -output_setting.field_map_size_x/2; x < output_setting.field_map_size_x/2; x += dx)
			{
				for (double z = -output_setting.field_map_size_z/2; z < output_setting.field_map_size_z/2; z += dz)
				{
					FieldEB field;
					calculate_fields(C0*(current_t - trigger_t), x, 0, z, laser, field, lua_state);
					write_field_maps_xz(current_t, x, 0, z, field);
				}
			}
			close_field_map_xz_files();
		}
	}

	if (output_setting.field_map_enable_yz)
	{
		#pragma omp parallel for default(shared)
		for (int t = 0; t < t_count; t++)
		{
			double current_t = t * dt;
			open_field_map_yz_files(output_dir, interaction, node, t);
			
			for (double y = -output_setting.field_map_size_y/2; y < output_setting.field_map_size_y/2; y += dy)
			{
				for (double z = -output_setting.field_map_size_z/2; z < output_setting.field_map_size_z/2; z += dz)
				{
					FieldEB field;
					calculate_fields(C0*(current_t - trigger_t), 0, y, z, laser, field, lua_state);
					write_field_maps_yz(current_t, 0, y, z, field);
				}
			}
			close_field_map_yz_files();
		}
	}
}	


void simulate (Simulation& simulation, OutputSetting& output_setting, Pulse& laser, Particle& particle, ParticleState& particle_state, Accellerator& accellerator, Node nodes[], lua::State* lua_state, fs::path& output_dir)
{
	
	printf("Simulation duration: %f s\n", simulation.duration * AU_TIME);
	
	// This is the laser period. Our simulation time will progress by units of this period. It will be splitted in integrations_laser and integrations_free subperiods.
	// TODO: this value must be cached because it does not change during all the simulation
	
	//
	
	RangeMode 	  current_range = FREE;
	unsigned int  current_interaction = 0;
	int 		  current_node = -1;
	double		  last_laser_enter_time = 0;
	double		  last_laser_exit_time  = 0;
	
	long double time_current = 0;
	while (time_current < simulation.duration)
	{
		printf("\rSimulating: %3.4f%%", (double)time_current / simulation.duration * 100);
		
		// Identifing if our particle is inside the laser action range or outside.
		// If outside we use the free motion laws, if inside we calculate the integration between the laser and the particle	
		int new_node = get_near_node_id(particle_state, accellerator.nodes, nodes, simulation.laser_influence_radius);
		
		if (current_range == LASER && new_node < 0 )
		{
			close_interaction_files();
			last_laser_exit_time = time_current;
			
			double laser_trigger_time = (last_laser_exit_time - last_laser_enter_time) / 2;
			
			simulate_field_maps(output_setting, current_interaction, current_node, last_laser_enter_time, last_laser_exit_time, laser_trigger_time, laser, lua_state, output_dir);
			
			
			plot_interaction_files(output_dir, current_interaction, current_node);
			plot_field_maps(output_dir, output_setting, current_interaction, current_node);
			
			
			current_range = FREE;
			current_node  = -1;
			current_interaction++;
		}
		else if (current_range == FREE && new_node != current_node)
		{
			current_range = LASER;
			current_node = new_node; 
			last_laser_enter_time = time_current;
			
			
			open_interaction_files(output_dir, current_interaction,  current_node);
		}
		
		
		if (current_range == LASER)
		{
			Node& node = nodes[current_node];
			simulate_laser(simulation, laser, node, particle, particle_state, time_current, current_interaction, lua_state);	
		}
		else
		{
			simulate_free (simulation,         	    particle, particle_state, time_current, accellerator.nodes, nodes,  lua_state);
		}
	}
	
	printf("\n");
}



	

