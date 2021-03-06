#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>
#include "simulator.hpp"
#include "type.hpp"
#include "util.hpp"

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
	FunctionFieldType*      function_field;

} gsl_odeiv_custom_params;

      
void calculate_fields(double pos_t, double pos_x, double pos_y, double pos_z, const Pulse& laser, Field& field, FunctionFieldType function_field)
{
	double param_time			=	pos_t/C0		* AU_TIME;
	double param_x				=	pos_x 			* AU_LENGTH;
	double param_y				=	pos_y 			* AU_LENGTH;
	double param_z				=	pos_z 			* AU_LENGTH;
  
	field = function_field(param_time, param_x, param_y, param_z, laser.params);

	field.e_x /= AU_ELECTRIC_FIELD; 
	field.e_y /= AU_ELECTRIC_FIELD;
	field.e_z /= AU_ELECTRIC_FIELD;
  
    field.b_x /= AU_MAGNETIC_FIELD;
	field.b_y /= AU_MAGNETIC_FIELD;
	field.b_z /= AU_MAGNETIC_FIELD; 

}
      
      
int gsl_odeiv_jac (double t, const double y[], double *dfdy, double dfdt[], void *params)
{
	cout << "jacobian used\n";
	return GSL_SUCCESS;
}

int gsl_odeiv_func_laser_fake(double t, const double y[], double f[], void *params)
{
	Particle& 	particle 				= *((gsl_odeiv_custom_params*)params)->particle;

	//double pos_t = C0*t;
	//double pos_x = y[0];
	//double pos_y = y[1];
	//double pos_z = y[2];
	double mom_x = y[3];
	double mom_y = y[4];
	double mom_z = y[5];
	
	// In out formulas we use B instead of H and we use the relation B = H/c₀
	// Formulas (3),(4),(5)
	
	// fac =  1/√(1 + p/(c₀m)²) = c₀/√(c₀²+p²)
	double fac = C0 / sqrt(C0 * C0 + mom_x * mom_x + mom_y * mom_y + mom_z * mom_z);
	
	f[0] = fac * mom_x / particle.rest_mass;
	f[1] = fac * mom_y / particle.rest_mass;
	f[2] = fac * mom_z / particle.rest_mass;
	f[3] = 0.d;
	f[4] = 0.d;
	f[5] = 0.d;

	return GSL_SUCCESS;
}

int gsl_odeiv_func_laser(double t, const double y[], double f[], void *params)
{
	Pulse& 	 	laser 	  				= *((gsl_odeiv_custom_params*)params)->laser;
	Particle& 	particle 				= *((gsl_odeiv_custom_params*)params)->particle;
	FunctionFieldType function_field    = *((gsl_odeiv_custom_params*)params)->function_field;

	double pos_t = C0*t;
	double pos_x = y[0];
	double pos_y = y[1];
	double pos_z = y[2];
	double mom_x = y[3];
	double mom_y = y[4];
	double mom_z = y[5];
	
	Field field;
	calculate_fields(pos_t, pos_x, pos_y, pos_z, laser, field, function_field);
	
	double e_x = field.e_x;
	double e_y = field.e_y;
	double e_z = field.e_z;
	
	double b_x = field.b_x;
	double b_y = field.b_y;
	double b_z = field.b_z;
	
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

void simulate_node_fake(
	Simulation& simulation, 
	Node& node,
	Particle& particle,
	ParticleStateLocal& state,
	double&  local_time_current,
	unsigned int interaction,
	SimluationResultNodeSummary& summary)
{
	// Trasforming global coordinates to local coordinates
	
	
	summary.local_time_enter = local_time_current;

	gsl_odeiv_custom_params params;
	params.laser 	  = NULL;
	params.particle	  = &particle;
	params.function_field = NULL;

	const gsl_odeiv_step_type* 	step_type 	= gsl_odeiv_step_rk8pd;
	gsl_odeiv_step* 			steps 		= gsl_odeiv_step_alloc(step_type, 6);
	gsl_odeiv_control* 			control		= gsl_odeiv_control_y_new (simulation.error_abs, simulation.error_rel);
	gsl_odeiv_evolve* 			evolve		= gsl_odeiv_evolve_alloc(6);
	gsl_odeiv_system 			system 		= {gsl_odeiv_func_laser_fake, gsl_odeiv_jac, 6, &params};

	
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
		field.e_x = 0.d;
		field.e_y = 0.d;
		field.e_z = 0.d;
		field.b_x = 0.d;
		field.b_y = 0.d;
		field.b_z = 0.d;
		
		SimluationResultNodeItem result;
		result.local_time		= local_time_current;
		result.local_state	= state;
		result.field	= field;
		summary.items.push_back(result);
		
		
		// Checking if we are outside the laser influence radius.
		if (!is_in_influence_radius(state, simulation.laser_influence_radius))
			break;
		
	}



	gsl_odeiv_evolve_free(evolve);
	gsl_odeiv_control_free(control);
	gsl_odeiv_step_free(steps);
	
	summary.local_time_exit = local_time_current;
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
	FunctionFieldType function_field)
{
	// Trasforming global coordinates to local coordinates
	
	
	summary.local_time_enter = local_time_current;

	gsl_odeiv_custom_params params;
	params.laser 	  = &laser;
	params.particle	  = &particle;
	params.function_field = &function_field;

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
		calculate_fields(local_time_current*C0, y[0], y[1], y[2], laser, field, function_field);
		
		if (on_node_time_progress != NULL) on_node_time_progress(simulation, laser, particle, state, interaction, node, local_time_current, field);
		
		SimluationResultNodeItem result;
		result.local_time		= local_time_current;
		result.local_state	= state;
		result.field	= field;
		summary.items.push_back(result);
		
		
		// Checking if we are outside the laser influence radius.
		if (!is_in_influence_radius(state, simulation.laser_influence_radius))
			break;
		
	}



	gsl_odeiv_evolve_free(evolve);
	gsl_odeiv_control_free(control);
	gsl_odeiv_step_free(steps);
	
	summary.local_time_exit = local_time_current;
}


void simulate_free(Simulation& simulation, Laboratory& laboratory, Particle& particle, ParticleStateGlobal& state, long double& global_time_current, FunctionFreeTimeProgress& on_free_time_progress, SimluationResultFreeSummary& summary)
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
	params.function_field = NULL;

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
	
	FunctionFieldType function_field)
{
	
	RangeMode 	  current_range = UNKN;
	unsigned int  current_interaction = 0;
	int 		  current_node = -1;
	
	long double time_current_global = 0;
	double      time_current_local;
	long double time_global_offset;
	
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
			
			if (current_range != UNKN)
				current_interaction++;
			current_range = FREE;
			current_node  = -1;
			
		}
		else if (current_range != NODE && new_node != current_node)
		{
			if (current_range == FREE && on_free_exit != NULL) on_free_exit(simulation, particle, particle_state_global, laboratory, time_current_global);
			
			current_range = NODE;
			current_node = new_node; 
			Node& node = laboratory.nodes[current_node];
			
			
			
			// We must to get the new local starting time. If the mode is enter it is very easy
			if (laser.timing_mode == ENTER)
			{
				time_current_local = laser.timing_offset;
			}
			else
			{
				// To know the correct time local we have to launch a simulation without laser and detect correct value.
				SimluationResultNodeSummary summary_fake;
				double time_current_local_fake = 0.d;
				ParticleStateLocal particle_state_local_fake;
				state_global_to_local(particle_state_local_fake, particle_state_global, node);
				
				simulate_node_fake(simulation, node, particle, particle_state_local_fake, time_current_local_fake, 0, summary_fake);
				
				// Now detecting the right time_current_local
				
				if (laser.timing_mode == NEAREST)
				{
					SimluationResultNodeItem nearest_item;
					nearest_item.local_time = 0.d;
					double nearest_distance = INFINITY;
					
					for (SimluationResultNodeItem item: summary_fake.items)
					{
						double current_distance = vector_module(item.local_state.position_x, item.local_state.position_y, item.local_state.position_z);
						if (current_distance < nearest_distance)
						{
							nearest_item = item;
							nearest_distance = current_distance;
						}
					}
					
					if (nearest_distance == INFINITY)
					{
						printf("ERROR - Unable to detect the nearest point for node %d\n", node.id);
						exit(-6);	
					}
					
					time_current_local = -(nearest_item.local_time-summary_fake.items.front().local_time) + laser.timing_offset;
				}
				else if (laser.timing_mode == EXIT)
				{
					time_current_local = -(summary_fake.items.back().local_time-summary_fake.items.front().local_time) + laser.timing_offset;
				}
			}
			
			time_global_offset = time_current_global - time_current_local;
			
			state_global_to_local(particle_state_local, particle_state_global, node);
			
			if (on_node_enter != NULL) on_node_enter(simulation, laser, particle,  particle_state_local, current_interaction, node, time_current_local);
		}
		
		
		if (current_range == NODE)
		{
			Node& node = laboratory.nodes[current_node];
			
			double before = time_current_local;
			
			SimluationResultNodeSummary summary;
			summary.node = node;
			summary.global_time_offset = time_global_offset;
			simulate_node(simulation, laser, node, particle, particle_state_local, time_current_local, current_interaction, on_node_time_progress, summary, function_field);
			summaries_node.push_back(summary);	
			state_local_to_global(particle_state_global, particle_state_local, node);
			
			
			time_current_global += time_current_local - before;
		}
		else if (current_range == FREE)
		{
			SimluationResultFreeSummary summary;
			simulate_free(simulation, laboratory, particle, particle_state_global, time_current_global, on_free_time_progress, summary);
			summaries_free.push_back(summary);
		}
	}
	
	if (current_range == FREE && on_free_exit != NULL) on_free_exit(simulation, particle, particle_state_global, laboratory, time_current_global);
	
	if (current_range == NODE)
	{
		if (on_node_exit != NULL) on_node_exit (simulation, laser, particle, particle_state_local, current_interaction, laboratory.nodes[current_node], time_current_local);
			state_local_to_global(particle_state_global, particle_state_local, laboratory.nodes[current_node]);
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
	FunctionFieldType function_field)
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
		function_field);
}

	

