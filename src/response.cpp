#include "response.hpp"
#include "type.hpp"
#include "util.hpp"


void error_attribute_unknown(string object, string attribute)
{
	printf("ERROR - unable to understand %s attribute '%s'\n", object.c_str(), attribute.c_str());
	exit(-1);	
}

string get_conversion_si_unit(string object, string attribute)
{
	if (object == "particle")
	{

		if (attribute == "position_x" || attribute == "position_y" || attribute == "position_z")
			return "m";
		else if (attribute == "position_phi" || attribute == "position_theta") 
			return "rad";
		else if (attribute == "position_rho")
			return "m";
		else if (attribute == "momentum_x" 	 || attribute == "momentum_y" || attribute == "momentum_z")
			return "Ns";
		else if (attribute == "momentum_phi" || attribute == "momentum_theta")
			return "rad";
		else if (attribute == "momentum_rho")
			return "Ns";
		else if (attribute == "energy_x" 	|| attribute == "energy_y" || attribute == "energy_z")
			return "J";
		else if (attribute == "energy_phi" 	|| attribute == "energy_theta")
			return "rad";
		else if (attribute == "energy_rho")
			return "J";
		else if (attribute == "rest_mass")
			return "Kg";
		else if (attribute == "charge")
			return "C";
	}
	else if (object == "laser")
	{
		if (attribute == "timing_offset")
			return "s";
		else
			return "arbitrary";
	}
	
	error_attribute_unknown(object, attribute);
	return "unknown";	
}

double get_conversion_si_value(string object, string attribute)
{
	if (object == "particle")
	{

		if (attribute == "position_x" || attribute == "position_y" || attribute == "position_z")
			return AU_LENGTH;
		else if (attribute == "position_phi" || attribute == "position_theta") 
			return 1.d;
		else if (attribute == "position_rho")
			return AU_LENGTH;
		else if (attribute == "momentum_x" || attribute == "momentum_y" || attribute == "momentum_z")
			return AU_MOMENTUM;
		else if (attribute == "momentum_phi" || attribute == "momentum_theta")
			return 1.d;
		else if (attribute == "momentum_rho")
			return AU_MOMENTUM;
		else if (attribute == "energy_x" || attribute == "energy_y" || attribute == "energy_z")
			return AU_ENERGY;
		else if (attribute == "energy_phi" || attribute == "energy_theta")
			return 1.d;
		else if (attribute == "energy_rho")
			return AU_ENERGY;
		else if (attribute == "rest_mass")
			return AU_MASS;
		else if (attribute == "charge")
			return AU_CHARGE;
	}
	else if (object == "laser")
	{
		if (attribute == "timing_offset")
			return AU_TIME;
		else
			return 1.d;
	}
	
	error_attribute_unknown(object, attribute);
	return 0.d;	
}

double get_attribute(Particle& particle, ParticleStateGlobal& particle_state, Pulse& laser, string object, string attribute)
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
			long double position_theta;
			long double position_phi;
			
			cartesian_to_spherical<long double>(particle_state.position_x,	particle_state.position_y, particle_state.position_z, position_theta, position_phi);
			
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

		else if (attribute == "energy_x")
			return momentum_to_energy_kinetic(particle.rest_mass, particle_state.momentum_x);
		else if (attribute == "energy_y")
			return momentum_to_energy_kinetic(particle.rest_mass, particle_state.momentum_y);
		else if (attribute == "energy_z")
			return momentum_to_energy_kinetic(particle.rest_mass, particle_state.momentum_z);
		else if (attribute == "energy_phi" || attribute == "energy_theta")
		{
			double momentum_theta;
			double momentum_phi;
			
			cartesian_to_spherical(particle_state.momentum_x, particle_state.momentum_y, particle_state.momentum_z, momentum_theta, momentum_phi);
			
			if (attribute == "energy_phi")
				return momentum_phi;
			else if (attribute == "energy_theta")
				return momentum_theta;
		}
		else if (attribute == "energy_rho")
			return momentum_to_energy_kinetic(particle.rest_mass, vector_module(particle_state.momentum_x,	particle_state.momentum_y, particle_state.momentum_z));
		else if (attribute == "rest_mass")
			 return particle.rest_mass;
		else if (attribute == "charge")
			 return particle.charge;
	}
	else if (object == "laser")
	{
		if (attribute == "timing_offset")
			 return laser.timing_offset;
		else if (laser.params_float.find(attribute) != laser.params_float.end())
			return laser.params_float[attribute];
	}
	
	error_attribute_unknown(object, attribute);
	return 0.d;
}

void set_attribute(Particle& particle, ParticleStateGlobal& particle_state, Pulse& laser, string object, string attribute, double new_value)
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
			long double position_theta;
			long double position_phi;
			long double position_rho;
			
			cartesian_to_spherical<long double>(particle_state.position_x,	particle_state.position_y, particle_state.position_z, position_theta, position_phi);
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
		else if (attribute == "momentum_phi" || attribute == "momentum_theta" || attribute == "momentum_rho")
		{
			double momentum_rho;
			double momentum_theta;
			double momentum_phi;
			
			cartesian_to_spherical<double>(particle_state.momentum_x,	particle_state.momentum_y, particle_state.momentum_z, momentum_theta, momentum_phi);
			momentum_rho = vector_module<double>(particle_state.position_x,	particle_state.position_y, particle_state.position_z);
			
			if (attribute == "momentum_phi")
				momentum_phi		= new_value;
			else if (attribute == "momentum_theta")
				momentum_theta	= new_value;
			else if (attribute == "momentum_rho")
				momentum_rho	= new_value;
			else
				error_attribute_unknown(object, attribute);
				
			spherical_to_cartesian(momentum_rho, momentum_theta, momentum_phi, particle_state.momentum_x, particle_state.momentum_y, particle_state.momentum_z);
		}
		else if (attribute == "energy_x")
			particle_state.momentum_x = energy_kinetic_to_momentum(particle.rest_mass, new_value);
		else if (attribute == "energy_y")
			particle_state.momentum_y = energy_kinetic_to_momentum(particle.rest_mass, new_value);
		else if (attribute == "energy_z")
			particle_state.momentum_z = energy_kinetic_to_momentum(particle.rest_mass, new_value);
		else if (attribute == "energy_phi" || attribute == "energy_theta" || attribute == "energy_rho")
		{
			double momentum_rho;
			double momentum_theta;
			double momentum_phi;
			
			cartesian_to_spherical<double>(particle_state.momentum_x,	particle_state.momentum_y, particle_state.momentum_z, momentum_theta, momentum_phi);
			momentum_rho = vector_module<double>(particle_state.position_x,	particle_state.position_y, particle_state.position_z);
			
			if (attribute == "energy_phi")
				momentum_phi	= new_value;
			else if (attribute == "energy_theta")
				momentum_theta	= new_value;
			else if (attribute == "energy_rho")
				momentum_rho	= energy_kinetic_to_momentum(particle.rest_mass, new_value);
			else
				error_attribute_unknown(object, attribute);
				
			spherical_to_cartesian(momentum_rho, momentum_theta, momentum_phi, particle_state.momentum_x, particle_state.momentum_y, particle_state.momentum_z);
		}
		else if (attribute == "rest_mass")
			 particle.rest_mass = new_value;
		else if (attribute == "charge")
			 particle.charge = new_value;
		else
			error_attribute_unknown(object, attribute);
	}
	else if (object == "laser")
	{	if (attribute == "timing_offset")
			laser.timing_offset = new_value;
		else if (laser.params_float.find(attribute) != laser.params_float.end())
			laser.params_float[attribute] = new_value;
		else
			error_attribute_unknown(object, attribute);
	}
	else
		error_attribute_unknown(object, attribute);

	
}


