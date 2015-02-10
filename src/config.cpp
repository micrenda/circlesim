#include <libconfig.h++>
#include <math.h>
#include <LuaState.h>
#include <boost/regex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/io/detail/quoted_manip.hpp>
#include "config.hpp"
#include "type.hpp"
#include "util.hpp"
#include "le.h"

using namespace libconfig;

void err_config()
{
    exit(-1);
}

void check_lua_error(lua::LoadError& e, string section_name, string fullcode)
{
	
	printf("Error while parsing '%s': %s\n", section_name.c_str(), e.what());
	printf("--------------------------------------------------------\n");
	printf("%s\n", fullcode.c_str());
	printf("--------------------------------------------------------\n");
	
	exit(-3);

}

bool missing_param(string param)
{
	printf("Missing or wrong format parameter: %s\n", param.c_str());
	exit(-1);
	return true;
}

bool wrong_param(string param, string message)
{
	printf("Wrong '%s' parameter value: %s\n", param.c_str(), message.c_str());
	exit(-1);
	return true;
}

void read_config(
	fs::path& cfg_file,
	Parameters& parameters,
	Simulation& simulation,
	OutputSetting& output,
	Pulse& laser,
	Particle& particle,
	ParticleState& particle_state,
	Accellerator& accellerator,
	Node nodes[], 
	lua::State* lua_state)
{
	Config* config = new Config;
	
	
	bo::unordered_map<string, int> 	  laser_field_param_map_int;
	bo::unordered_map<string, long>   laser_field_param_map_int64;
	bo::unordered_map<string, double> laser_field_param_map_float;
	bo::unordered_map<string, string> laser_field_param_map_string;
	bo::unordered_map<string, bool>	  laser_field_param_map_boolean;


	// Read the file. If there is an error, report it and exit.
	try 
	{
		config->readFile(cfg_file.c_str());

		Setting&  config_simulation	 		= config->lookup("simulation");
		Setting&  config_laser 				= config->lookup("laser");
		Setting&  config_particle 			= config->lookup("particle");
		Setting&  config_accellerator 		= config->lookup("accellerator");
		Setting&  config_output				= config->lookup("output");

		config_laser.lookupValue			("duration",  	parameters.pulse_duration)							|| missing_param("duration (laser)");
		
		config_laser.lookupValue			("func_fields",	parameters.func_fields)								|| missing_param("func_fields");
		
		static const bo::regex e("^func_param_([A-Za-z\\_0-9]+)$");
		for (int i = 0; i < config_simulation.getLength(); i++)
		{
			Setting& setting = config_laser[i];
			
			bo::match_results<string::const_iterator> what;
			if (bo::regex_match(string(setting.getName()), what, e))
			{
				switch (setting.getType())
				{
					case Setting::TypeInt:
						laser_field_param_map_int[what[1]] 		= setting;
					break;
					
					case Setting::TypeInt64:
						laser_field_param_map_int64[what[1]] 	= setting;
					break;
					
					case Setting::TypeFloat:
						laser_field_param_map_float[what[1]] 	= setting;
					break;
					
					case Setting::TypeString:
						laser_field_param_map_string[what[1]] 	= (const char *) setting;
					break;
					
					case Setting::TypeBoolean:
						laser_field_param_map_boolean[what[1]] 	= setting;
					break;
					
					default:
						wrong_param(config_simulation.getName(), "Accepted types are: TypeInt, TypeInt64, TypeFloat, TypeString, TypeBoolean\n");
						exit(-1);
					break;
				
				}
			}
		}
		

		config_particle.lookupValue			("rest_mass",  				parameters.rest_mass)					|| missing_param("rest_mass");
		config_particle.lookupValue			("charge",  				parameters.charge)						|| missing_param("charge");
		
		config_particle.lookupValue			("initial_reference_node",  parameters.initial_reference_node)	|| missing_param("initial_reference_node");

		parameters.has_position_cart = config_particle.exists("initial_position_x") 		&& config_particle.exists("initial_position_y") 	&& config_particle.exists("initial_position_z");
		parameters.has_position_sphe = config_particle.exists("initial_position_module") 	&& config_particle.exists("initial_position_theta")	&& config_particle.exists("initial_position_phi");
		parameters.has_momentum_cart = config_particle.exists("initial_momentum_x") 		&& config_particle.exists("initial_momentum_y") 	&& config_particle.exists("initial_momentum_z");
		parameters.has_momentum_sphe = config_particle.exists("initial_momentum_module") 	&& config_particle.exists("initial_momentum_theta")	&& config_particle.exists("initial_momentum_phi");

		if (parameters.has_position_cart)
		{
			config_particle.lookupValue	("initial_position_x",  		parameters.initial_position_x)	|| missing_param("initial_position_x");
			config_particle.lookupValue	("initial_position_y",  		parameters.initial_position_y)	|| missing_param("initial_position_y");
			config_particle.lookupValue	("initial_position_z",  		parameters.initial_position_z)	|| missing_param("initial_position_z");
		}
		
		if (parameters.has_position_sphe)
		{
			config_particle.lookupValue	("initial_position_module",		parameters.initial_position_module)	|| missing_param("initial_position_module");
			config_particle.lookupValue	("initial_position_theta",		parameters.initial_position_theta)	|| missing_param("initial_position_theta");
			config_particle.lookupValue	("initial_position_phi",  		parameters.initial_position_phi)	|| missing_param("initial_position_phi");
		}
		
		if (parameters.has_momentum_cart)
		{
			config_particle.lookupValue	("initial_momentum_x",  		parameters.initial_momentum_x)	|| missing_param("initial_momentum_x");
			config_particle.lookupValue	("initial_momentum_y",  		parameters.initial_momentum_y)	|| missing_param("initial_momentum_y");
			config_particle.lookupValue	("initial_momentum_z",  		parameters.initial_momentum_z)	|| missing_param("initial_momentum_z");
		}
		
		if (parameters.has_momentum_sphe)
		{
			config_particle.lookupValue	("initial_momentum_module",		parameters.initial_momentum_module)	|| missing_param("initial_momentum_module");
			config_particle.lookupValue	("initial_momentum_theta",		parameters.initial_momentum_theta)	|| missing_param("initial_momentum_theta");
			config_particle.lookupValue	("initial_momentum_phi",  		parameters.initial_momentum_phi)	|| missing_param("initial_momentum_phi");
		}


		config_simulation.lookupValue		("basename",  				parameters.basename)				|| missing_param("basename");
		config_simulation.lookupValue		("error_abs",  				parameters.error_abs)				|| missing_param("error_abs");
		config_simulation.lookupValue		("error_rel",  				parameters.error_rel)				|| missing_param("error_rel");
		config_simulation.lookupValue		("time_resolution_laser",	parameters.time_resolution_laser)	|| missing_param("time_resolution_laser");
		config_simulation.lookupValue		("time_resolution_free",	parameters.time_resolution_free)	|| missing_param("time_resolution_free");
		config_simulation.lookupValue		("duration",  				parameters.simulation_duration)		|| missing_param("duration (simulation)");
		config_simulation.lookupValue		("laser_influence_radius",  parameters.laser_influence_radius)	|| missing_param("laser_influence_radius");
		config_simulation.lookupValue		("func_commons",  			parameters.func_commons)			|| missing_param("func_commons");

		config_accellerator.lookupValue		("nodes",   				parameters.nodes)					|| missing_param("nodes");
		config_accellerator.lookupValue		("radius",  				parameters.radius)					|| missing_param("radius");

		config_accellerator.lookupValue		("node_axis_mode",  		parameters.node_axis_mode)			|| missing_param("node_axis_mode");
		config_accellerator.lookupValue		("node_axis_rotate_theta",  parameters.node_axis_rotate_theta)	|| missing_param("node_axis_rotate_theta");
		config_accellerator.lookupValue		("node_axis_rotate_phi",  	parameters.node_axis_rotate_phi)	|| missing_param("node_axis_rotate_phi");

		config_accellerator.lookupValue 	("timing_mode",   			parameters.timing_mode)				|| missing_param("timing_mode");
		config_accellerator.lookupValue		("timing_value",  			parameters.timing_value)			|| missing_param("timing_value");
		
		config_output.lookupValue			("field_map_enable_xy",		parameters.field_map_enable_xy)		|| missing_param("field_map_enable_xy");
		config_output.lookupValue			("field_map_enable_xz",		parameters.field_map_enable_xz)		|| missing_param("field_map_enable_xz");
		config_output.lookupValue			("field_map_enable_yz",		parameters.field_map_enable_yz)		|| missing_param("field_map_enable_yz");
		
		config_output.lookupValue			("field_map_resolution_t",	parameters.field_map_resolution_t)	|| missing_param("field_map_resolution_t");
		config_output.lookupValue			("field_map_resolution_x",	parameters.field_map_resolution_x)	|| missing_param("field_map_resolution_x");
		config_output.lookupValue			("field_map_resolution_y",	parameters.field_map_resolution_y)	|| missing_param("field_map_resolution_y");
		config_output.lookupValue			("field_map_resolution_z",	parameters.field_map_resolution_z)	|| missing_param("field_map_resolution_z");
		
		config_output.lookupValue			("field_map_size_x",		parameters.field_map_size_x)		|| missing_param("field_map_size_x");
		config_output.lookupValue			("field_map_size_y",		parameters.field_map_size_y)		|| missing_param("field_map_size_y");
		config_output.lookupValue			("field_map_size_z",		parameters.field_map_size_z)		|| missing_param("field_map_size_z");
	  
	}
	catch (ParseException& e)  
	{
		printf("Error while reading configuration file: %s\n", e.getFile());
		printf("Line %d: %s\n", e.getLine(), e.getError());
		exit(-1);
	}
	
	
	// Trasforming all the read parameters and converting (if needed) the measure units
	
	simulation.basename 				= parameters.basename;
	simulation.error_abs				= parameters.error_abs;
	simulation.error_rel				= parameters.error_rel;
	simulation.time_resolution_laser	= parameters.time_resolution_laser	/ AU_TIME;
	simulation.time_resolution_free		= parameters.time_resolution_free	/ AU_TIME;
	simulation.laser_influence_radius	= parameters.laser_influence_radius	/ AU_LENGTH;
	simulation.duration					= parameters.simulation_duration 	/ AU_TIME;
	
	laser.duration						= parameters.pulse_duration 		/ AU_TIME;
	
	
	// Loading the common functins. Loading and evaluating so remain availabe to the other LUA scripts
	lua_state->doString(parameters.func_commons);
	//check_lua_error(lua_state, err, "func_commons", parameters.func_commons);
	// Loading other lua scripts
	
	string s = "function func_fields(D, t, x, y, z)\n";
	s += "    -- Injecting external variables\n";
	for (bo::unordered_map<string,int>::iterator	entry = laser_field_param_map_int.begin(); entry != laser_field_param_map_int.end(); ++entry) 
		s += (bo::format("    % -12s\t= %d\n") 		% entry->first % entry->second).str();
	for (bo::unordered_map<string,long>::iterator	entry = laser_field_param_map_int64.begin(); entry != laser_field_param_map_int64.end(); ++entry) 
		s += (bo::format("    % -12s\t= %l\n") 		% entry->first % entry->second).str();
	for (bo::unordered_map<string,double>::iterator	entry = laser_field_param_map_float.begin(); entry != laser_field_param_map_float.end(); ++entry) 
		s += (bo::format("    % -12s\t= %.16E\n") 	% entry->first % entry->second).str();
	for (bo::unordered_map<string,string>::iterator	entry = laser_field_param_map_string.begin(); entry != laser_field_param_map_string.end(); ++entry) 
		s += (bo::format("    % -12s\t= %s\n") 		% entry->first %  bo::io::quoted(entry->second)).str();
	for (bo::unordered_map<string,bool>::iterator	entry = laser_field_param_map_boolean.begin(); entry != laser_field_param_map_boolean.end(); ++entry) 
		s += (bo::format("    % -12s\t= %s\n") 		% entry->first % (entry->second ? "true" : "false")).str();
	s += "    -- completed\n\n";
	
	s += parameters.func_fields + "\n";
	s += "end\n";
	
	try
	{
		lua_state->doString(s.c_str());
	}
	catch (lua::LoadError& e)
	{
		check_lua_error(e, "func_fields", s);
	}
	
	printf("Generated fields LUA function:\n");
	printf("--------------------------------------------------------\n");
	printf("%s\n", s.c_str());
	printf("--------------------------------------------------------\n");

	particle.rest_mass 					= parameters.rest_mass 				/ AU_MASS;
	particle.charge 					= parameters.charge    				/ AU_CHARGE;
	
	accellerator.nodes					= parameters.nodes;
	accellerator.radius					= parameters.radius 				/ AU_LENGTH;
	
	if (ba::iequals(parameters.node_axis_mode, 			"MODE_P")) 
		accellerator.node_axis_mode 	= MODE_P;
	else if (ba::iequals(parameters.node_axis_mode, 	"MODE_T_FW"))
		accellerator.node_axis_mode 	= MODE_T_FW;
	else if (ba::iequals(parameters.node_axis_mode, 	"MODE_T_BW")) 
		accellerator.node_axis_mode 	= MODE_T_BW;
	else
		wrong_param("node_axis_mode", "accepted values are: MODE_P, MODE_T_FW, MODE_T_BW");
		
	accellerator.node_axis_rotate_theta	= parameters.node_axis_rotate_theta;
	accellerator.node_axis_rotate_phi	= parameters.node_axis_rotate_phi;
	
	
	if (ba::iequals(parameters.timing_mode, 		"TURN_ON")) 
		accellerator.timing_mode = TURN_ON;
	else if (ba::iequals(parameters.timing_mode, 	"CONSTANT"))
		accellerator.timing_mode = CONSTANT;
	else if (ba::iequals(parameters.timing_mode, 	"TURN_OFF")) 
		accellerator.timing_mode = TURN_OFF;
	else
		accellerator.timing_mode = CONSTANT;
	
	accellerator.timing_value			= parameters.timing_value;
	
	output.field_map_enable_xy			= parameters.field_map_enable_xy;
	output.field_map_enable_xz			= parameters.field_map_enable_xz;
	output.field_map_enable_yz			= parameters.field_map_enable_yz;
	
	output.field_map_resolution_t	= parameters.field_map_resolution_t  / AU_TIME;
	output.field_map_resolution_x	= parameters.field_map_resolution_x  / AU_LENGTH;
	output.field_map_resolution_y	= parameters.field_map_resolution_y  / AU_LENGTH;
	output.field_map_resolution_z	= parameters.field_map_resolution_z  / AU_LENGTH;
	
	output.field_map_size_x	= parameters.field_map_size_x / AU_LENGTH;
	output.field_map_size_y	= parameters.field_map_size_y / AU_LENGTH;
	output.field_map_size_z	= parameters.field_map_size_z / AU_LENGTH;

	delete config;
}


void init_nodes(Accellerator& accellerator, Node nodes[])
{
	
	for (unsigned int n = 0; n < accellerator.nodes; n++)
	{
		Node& node = nodes[n];
		
		node.id = n;
		
		node.position_x = accellerator.radius * sin(2 * M_PI * n / accellerator.nodes);
		node.position_y = accellerator.radius * cos(2 * M_PI * n / accellerator.nodes);
		node.position_z = 0;
		
		
		double theta	= 0; 
		double phi		= 0;
		
		if (accellerator.node_axis_rotate_theta)
			theta = +2 * M_PI * n / accellerator.nodes;
		if (accellerator.node_axis_rotate_phi)
			theta = -2 * M_PI * n / accellerator.nodes;
		
		mat versors = zeros<mat>(3,3);
		
		switch (accellerator.node_axis_mode)
		{
			case MODE_P:
				versors(0,0) =  1; 
				versors(1,0) =  0; 
				versors(2,0) =  0;
				
				versors(0,1) =  0; 
				versors(1,1) =  1; 
				versors(2,1) =  0;
				
				versors(0,2) =  0; 
				versors(1,2) =  0; 
				versors(2,2) =  1;
			break;
			
			
			case MODE_T_FW:
				versors(0,0) =  0; 
				versors(1,0) =  1; 
				versors(2,0) =  0;
				
				versors(0,1) =  0; 
				versors(1,1) =  0; 
				versors(2,1) =  1;
				
				versors(0,2) =  1; 
				versors(1,2) =  0; 
				versors(2,2) =  0;
			break;
			
			case MODE_T_BW:
				versors(0,0) =  0; 
				versors(1,0) = -1; 
				versors(2,0) =  0;
				
				versors(0,1) =  0; 
				versors(1,1) =  0; 
				versors(2,1) =  1;
				
				versors(0,2) = -1; 
				versors(1,2) =  0; 
				versors(2,2) =  0;
			break;

		}
		
		rotate_spherical(versors(0,0), versors(1,0), versors(2,0), theta, phi);
		rotate_spherical(versors(0,1), versors(1,1), versors(2,1), theta, phi);
		rotate_spherical(versors(0,2), versors(1,2), versors(2,2), theta, phi);
		
		// setting to zero all the versor components where abs(v) < 1E-15
		// We do this because these values are likely created by cos(π/2) != 0 and sin(0) != 0functions and
   
		if (abs(versors(0,0)) < 1E-15)  versors(0,0) =  0; 
		if (abs(versors(1,0)) < 1E-15)  versors(1,0) =  0; 
		if (abs(versors(2,0)) < 1E-15)  versors(2,0) =  0; 
		if (abs(versors(0,1)) < 1E-15)  versors(0,1) =  0; 
		if (abs(versors(1,1)) < 1E-15)  versors(1,1) =  0; 
		if (abs(versors(2,1)) < 1E-15)  versors(2,1) =  0; 
		if (abs(versors(0,2)) < 1E-15)  versors(0,2) =  0; 
		if (abs(versors(1,2)) < 1E-15)  versors(1,2) =  0; 
		if (abs(versors(2,2)) < 1E-15)  versors(2,2) =  0; 
			
		
		node.axis = versors;
		
#ifdef DEBUG_NODE_VERSORS
		printf("Node %d\n", node.id);
		node.axis.print("local versors:");
		printf("\n\n");
#endif

	}
}


void init_position_and_momentum(Parameters& parameters, Accellerator& accellerator, ParticleState& particle_state, Node nodes[])
{
	// Setting the position of the particle relative to selected node
	
	if (parameters.initial_reference_node < 0 || parameters.initial_reference_node >= (int)accellerator.nodes)
	{
		wrong_param("initial_reference_node", "Invalid node index\n");
	}
		
	Node& first_node = nodes[parameters.initial_reference_node];
	
	
	// Initializing POSITION 
	double rel_pos_x;
	double rel_pos_y;
	double rel_pos_z;
	
	if (parameters.has_position_sphe && parameters.has_position_cart)
	{
		wrong_param("initial_position", "You must to specify one of spherical or cartesian position. You can not specify both.\n");
	}
	else if (parameters.has_position_sphe)
	{

		spherical_to_cartesian(
			parameters.initial_position_module,
			parameters.initial_position_theta,
			parameters.initial_position_phi,
			rel_pos_x,
			rel_pos_y,
			rel_pos_z);
		
		
		rotate_spherical(rel_pos_x, rel_pos_y, rel_pos_z, parameters.initial_position_theta, parameters.initial_position_phi);
		
	}
	else if (parameters.has_position_cart)
	{
		rel_pos_x	= parameters.initial_position_x / AU_LENGTH;
		rel_pos_y	= parameters.initial_position_y / AU_LENGTH;
		rel_pos_z	= parameters.initial_position_z / AU_LENGTH;
	}
	else
	{
		wrong_param("initial_position", "You must to specify an initial position (cartesian or spherical).\n");
	}
	
	
	// Initializing MOMENTUM 
	double rel_mom_x;
	double rel_mom_y;
	double rel_mom_z;
	
	if (parameters.has_momentum_sphe && parameters.has_momentum_cart)
	{
		wrong_param("initial_momentum", "You must to specify one of spherical or cartesian momentum. You can not specify both.\n");
	}
	else if (parameters.has_momentum_sphe)
	{
		spherical_to_cartesian(
			parameters.initial_momentum_module,
			parameters.initial_momentum_theta,
			parameters.initial_momentum_phi,
			rel_mom_x,
			rel_mom_y,
			rel_mom_z);
			
		rotate_spherical(rel_mom_x, rel_mom_y, rel_mom_z, parameters.initial_momentum_theta, parameters.initial_momentum_phi);
	}
	else if (parameters.has_momentum_cart)
	{
		rel_mom_x	= parameters.initial_momentum_x / AU_MOMENTUM;
		rel_mom_y	= parameters.initial_momentum_y / AU_MOMENTUM;
		rel_mom_z	= parameters.initial_momentum_z / AU_MOMENTUM;
	}
	else
	{
		wrong_param("initial_momentum", "You must to specify an initial momentum (cartesian or spherical).\n");
	}	
		
		
		
	
	// Converting to global state
	state_local_to_global(particle_state, first_node, rel_pos_x, rel_pos_y, rel_pos_z, rel_mom_x, rel_mom_y, rel_mom_z);
}
