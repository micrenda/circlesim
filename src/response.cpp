#include "response.hpp"
#include "type.hpp"
#include "util.hpp"


void error_attribute_unknown(string object, string attribute)
{
	printf("ERROR - unable to understand %s attribute '%s'\n", object.c_str(), attribute.c_str());
	exit(-1);	
}

double get_attribute(Node& node, Particle& particle, ParticleStateLocal& particle_state, Pulse& laser, string object, string attribute)
{
	if (object == "particle")
	{

		if (attribute == "position_x")
			return particle_state.position_x;
		else if (attribute == "position_y")
			return particle_state.position_y;
		else if (attribute == "position_z")
			return particle_state.position_z;
		else if (attribute == "position_phi" || attribute == "position_theta") 
		{
			double position_theta;
			double position_phi;
			
			cartesian_to_spherical(particle_state.position_x,	particle_state.position_y, particle_state.position_z, position_theta, position_phi);
			
			if (attribute == "position_phi")
				return position_phi;
			else if (attribute == "position_theta")
				return position_theta;
		}
		else if (attribute == "position_rho")
			return vector_module(particle_state.position_x,	particle_state.position_y, particle_state.position_z);
		else if (attribute == "momentum_x")
			return particle_state.momentum_x;
		else if (attribute == "momentum_y")
			return particle_state.momentum_y;
		else if (attribute == "momentum_z")
			return particle_state.momentum_z;
		else if (attribute == "momentum_phi" || attribute == "momentum_theta")
		{
			double momentum_theta;
			double momentum_phi;
			
			cartesian_to_spherical(particle_state.momentum_x, particle_state.momentum_y, particle_state.momentum_z, momentum_theta, momentum_phi);
			
			if (attribute == "momentum_phi")
				return momentum_phi;
			else if (attribute == "momentum_theta")
				return momentum_theta;
		}
		else if (attribute == "momentum_rho")
			return vector_module(particle_state.momentum_x,	particle_state.momentum_y, particle_state.momentum_z);
		else if (attribute == "energy")
			return momentum_to_energy(particle.rest_mass, vector_module(particle_state.momentum_x,	particle_state.momentum_y, particle_state.momentum_z));
		else if (attribute == "rest_mass")
			 return particle.rest_mass;
		else if (attribute == "charge")
			 return particle.charge;
	}
	else if (object == "particle")
	{
		if (attribute == "duration")
			return laser.duration;
		else if (laser.params_float.find(attribute) != laser.params_float.end())
			return laser.params_float[attribute];
	}
	
	error_attribute_unknown(object, attribute);
	return 0.d;
}

void set_attribute(Particle& particle, ParticleStateLocal& particle_state, Pulse& laser, string object, string attribute, double new_value)
{
	if (object == "particle")
	{

		if (attribute == "position_x")
			particle_state.position_x = new_value;
		else if (attribute == "position_y")
			particle_state.position_y = new_value;
		else if (attribute == "position_z")
			particle_state.position_z = new_value;
		else if (attribute == "position_phi" || attribute == "position_theta" || attribute == "position_rho") 
		{
			double position_theta;
			double position_phi;
			double position_rho;
			
			cartesian_to_spherical(particle_state.position_x,	particle_state.position_y, particle_state.position_z, position_theta, position_phi);
			position_rho = vector_module(particle_state.position_x,	particle_state.position_y, particle_state.position_z);
			
			if (attribute == "position_phi")
				position_phi		= new_value;
			else if (attribute == "position_theta")
				position_theta	= new_value;
			else if (attribute == "position_rho")
				position_rho	= new_value;
			else
				error_attribute_unknown(object, attribute);
				
			spherical_to_cartesian(position_rho, position_theta, position_phi, particle_state.position_x, particle_state.position_y, particle_state.position_z);
		}
		else if (attribute == "momentum_x")
			particle_state.momentum_x = new_value;
		else if (attribute == "momentum_y")
			particle_state.momentum_y = new_value;
		else if (attribute == "momentum_z")
			particle_state.momentum_z = new_value;
		else if (attribute == "momentum_phi" || attribute == "momentum_theta" || attribute == "momentum_rho" || attribute == "energy")
		{
			double momentum_rho;
			double momentum_theta;
			double momentum_phi;
			
			cartesian_to_spherical(particle_state.momentum_x,	particle_state.momentum_y, particle_state.momentum_z, momentum_theta, momentum_phi);
			momentum_rho = vector_module(particle_state.position_x,	particle_state.position_y, particle_state.position_z);
			
			if (attribute == "momentum_phi")
				momentum_phi		= new_value;
			else if (attribute == "momentum_theta")
				momentum_theta	= new_value;
			else if (attribute == "momentum_rho")
				momentum_rho	= new_value;
			else if (attribute == "energy")
				momentum_rho	= energy_to_momentum(particle.rest_mass, new_value);
			else
				error_attribute_unknown(object, attribute);
				
			spherical_to_cartesian(momentum_rho, momentum_theta, momentum_phi, particle_state.position_x, particle_state.position_y, particle_state.position_z);
		}
		else if (attribute == "rest_mass")
			 particle.rest_mass = new_value;
		else if (attribute == "charge")
			 particle.charge = new_value;
		else
			error_attribute_unknown(object, attribute);
	}
	else if (object == "laser")
	{
		if (attribute == "duration")
			laser.duration = new_value;
		else if (laser.params_float.find(attribute) != laser.params_float.end())
			laser.params_float[attribute] = new_value;
		else
			error_attribute_unknown(object, attribute);
	}
	else
		error_attribute_unknown(object, attribute);

	
}


