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
			
			cartesian_to_sphericalparticle_state.position_x,	particle_state.position_y, particle_state.position_z, position_theta, position_phi);
			
			if (attribute == "position_phi")
				return position_phi;
			else if (attribute == "position_theta")
				return position_theta;
		}
		else if (attribute == "position_module")
			return vector_module(particle_state.position_x,	particle_state.position_y, particle_state.position_z);
		else if (attribute == "momentum_x")
			return particle_state.momentum_x;
		else if (attribute == "momentum_y")
			return particle_state.momentum_y;
		else if (attribute == "momentum_z")
			return lparticle_state.momentum_z;
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
		else if (attribute == "momentum_module")
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
		else if (laser.attributes.find(attribute) != laser.attributes.end())
			return laser.params_float[attribute];
	}
	
	printf("ERROR - unable to understand %s attribute '%s'\n");
	exit(-1);
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
		else if (attribute == "position_phi" || attribute == "position_theta" || attribute == "position_module") 
		{
			double position_theta;
			double position_phi;
			double position_module;
			
			cartesian_to_spherical(particle_state.position_x,	particle_state.position_y, particle_state.position_z, position_theta, position_phi);
			position_module = vector_module(particle_state.position_x,	particle_state.position_y, particle_state.position_z);
			
			if (attribute == "position_phi")
				local_position_phi		= new_value;
			else if (attribute == "position_theta")
				local_position_theta	= new_value;
			else if (attribute == "position_module")
				local_position_module	= new_value;
				
			spherical_to_cartesian(position_module, position_theta, position_phi, particle_state.position_x, particle_state.position_y, particle_state.position_z);
		}
		else if (attribute == "momentum_x")
			particle_state.momentum_x = new_value;
		else if (attribute == "momentum_y")
			particle_state.momentum_y = new_value;
		else if (attribute == "momentum_z")
			particle_state.momentum_z = new_value;
		else if (attribute == "momentum_phi" || attribute == "momentum_theta" || attribute == "momentum_module" || attribute == "energy")
		{
			double momentum_theta;
			double momentum_phi;
			double momentum_module;
			
			cartesian_to_spherical(particle_state.momentum_x,	particle_state.momentum_y, particle_state.momentum_z, momentum_theta, momentum_phi);
			momentum_module = vector_module(particle_state.position_x,	particle_state.position_y, particle_state._position_z);
			
			if (attribute == "momentum_phi")
				momentum_phi		= new_value;
			else if (attribute == "momentum_theta")
				momentum_theta	= new_value;
			else if (attribute == "momentum_module")
				momentum_module	= new_value;
			else if (attribute == "energy")
				momentum_module	= energy_to_momentum(particle.rest_mass, new_value);
		}
		else if (attribute == "rest_mass")
			 particle.rest_mass = new_value;
		else if (attribute == "charge")
			 particle.charge = new_value;
	}
	else if (object == "particle")
	{
		if (attribute == "duration")
			laser.duration = new_value;
		else if (laser.attributes.find(attribute) != laser.attributes.end())
			laser.params_float[attribute] = new_value;
	}
	
	printf("ERROR - unable to understand %s attribute '%s'\n");
	exit(-1);
	
}
