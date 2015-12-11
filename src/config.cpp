#include <libconfig.h++>
#include <math.h>
#include <omp.h>
#include <boost/regex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include "config.hpp"
#include "type.hpp"
#include "util.hpp"
#include "response.hpp"

using namespace libconfig;

void err_config()
{
    exit(-1);
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


void inject_laser_variables(string& s, Pulse& laser, bool hide_unused)
{
	
	string v_attr = "";
	if (hide_unused)
		v_attr = "__attribute__ ((unused))";
	
	s += "    // Injecting laser attributes\n";
	for (map<string,int>::iterator	entry = laser.params.params_int.begin(); entry != laser.params.params_int.end(); ++entry) 
		s += (bo::format("    const int    %s % -12s\t= %d;\n") 		% v_attr % entry->first % entry->second).str();
	for (map<string,long>::iterator	entry = laser.params.params_int64.begin(); entry != laser.params.params_int64.end(); ++entry) 
		s += (bo::format("    const long   %s % -12s\t= %l;\n") 		% v_attr % entry->first % entry->second).str();
	for (map<string,double>::iterator	entry = laser.params.params_float.begin(); entry != laser.params.params_float.end(); ++entry) 
		s += (bo::format("    const double %s % -12s\t= %.16E;\n") 	% v_attr % entry->first % entry->second).str();
	for (map<string,string>::iterator	entry = laser.params.params_string.begin(); entry != laser.params.params_string.end(); ++entry)
	{
		string value = entry->second;
		bo::replace_all(value, "\"", "\\\"");
		s += (bo::format("    const string %s % -12s\t= %s;\n") 		% v_attr % entry->first % value).str();
	}
	for (map<string,bool>::iterator	entry = laser.params.params_boolean.begin(); entry != laser.params.params_boolean.end(); ++entry) 
		s += (bo::format("    const bool   %s % -12s\t= %s;\n") 		% v_attr % entry->first % (entry->second ? "true" : "false")).str();
	s += "    // completed\n\n";	
	
}

void link_laser_variables(string& s, Pulse& laser, bool hide_unused)
{
	
	string v_attr = "";
	if (hide_unused)
		v_attr = "__attribute__ ((unused))";
	
	s += "    // Linking laser attributes\n";
	for (map<string,int>::iterator	entry = laser.params.params_int.begin(); entry != laser.params.params_int.end(); ++entry) 
		s += (bo::format("    const int&    %s % -12s\t= params.params_int.at(\"%s\");\n") 		% v_attr % entry->first % entry->first).str();
	for (map<string,long>::iterator	entry = laser.params.params_int64.begin(); entry != laser.params.params_int64.end(); ++entry) 
		s += (bo::format("    const long&   %s % -12s\t= params.params_int64.at(\"%s\");\n") 		% v_attr % entry->first % entry->first).str();
	for (map<string,double>::iterator	entry = laser.params.params_float.begin(); entry != laser.params.params_float.end(); ++entry) 
		s += (bo::format("    const double& %s % -12s\t= params.params_float.at(\"%s\");\n") 	% v_attr % entry->first 	% entry->first).str();
	for (map<string,string>::iterator	entry = laser.params.params_string.begin(); entry != laser.params.params_string.end(); ++entry)
		s += (bo::format("    const string& %s % -12s\t= params.params_string.at(\"%s\");\n") 		% v_attr % entry->first % entry->first).str();
	for (map<string,bool>::iterator	entry = laser.params.params_boolean.begin(); entry != laser.params.params_boolean.end(); ++entry) 
		s += (bo::format("    const bool&   %s % -12s\t= params.params_boolean.at(\"%s\");\n") 		% v_attr % entry->first % entry->first).str();
	s += "    // completed\n\n";	
	
}

void init_position_and_momentum(Parameters& parameters, Particle& particle, Laboratory& laboratory, ParticleStateGlobal& state_global)
{

	
	if (parameters.has_position_sphe && parameters.has_position_cart)
	{
		wrong_param("initial_position", "You must to specify one of spherical or cartesian position. You can not specify both.\n");
	}
	else if (parameters.has_position_sphe)
	{
		spherical_to_cartesian<long double>(
			parameters.initial_position_rho / AU_LENGTH,
			parameters.initial_position_theta,
			parameters.initial_position_phi,
			state_global.position_x,
			state_global.position_y,
			state_global.position_z);
	}
	else if (parameters.has_position_cart)
	{
		state_global.position_x	= parameters.initial_position_x / AU_LENGTH;
		state_global.position_y	= parameters.initial_position_y / AU_LENGTH;
		state_global.position_z	= parameters.initial_position_z / AU_LENGTH;
	}
	else
	{
		wrong_param("initial_position", "You must to specify an initial position (cartesian or spherical).\n");
	}
	
	
	// Initializing MOMENTUM 
	short count = 0;
	
	if (parameters.has_momentum_sphe) 	count++;
	if (parameters.has_momentum_cart) 	count++;
	if (parameters.has_energy_sphe) 	count++;
	if (parameters.has_energy_cart) 	count++;
	
	if (count > 1)
	{
		wrong_param("initial_momentum", "You must to specify one of: spherical momentum, cartesian momentum, spherical energy, cartesian energy. You can not specify more of them in the same time.\n");
	}
	else if (count == 0)
	{
		wrong_param("initial_momentum", "You must to specify an initial momentum (cartesian or spherical) or an initial energy (cartesian or spherical).\n");
	}
	else if (parameters.has_momentum_sphe)
	{
		spherical_to_cartesian<double>(
			parameters.initial_momentum_rho / AU_MOMENTUM,
			parameters.initial_momentum_theta,
			parameters.initial_momentum_phi,
			state_global.momentum_x,
			state_global.momentum_y,
			state_global.momentum_z);
	}
	else if (parameters.has_momentum_cart)
	{
		state_global.momentum_x	= parameters.initial_momentum_x / AU_MOMENTUM;
		state_global.momentum_y	= parameters.initial_momentum_y / AU_MOMENTUM;
		state_global.momentum_z	= parameters.initial_momentum_z / AU_MOMENTUM;
	}
	else if (parameters.has_energy_sphe)
	{
		double momentum_rho 	= energy_kinetic_to_momentum(particle.rest_mass, parameters.initial_energy_rho / AU_ENERGY);
		double momentum_theta	= parameters.initial_energy_theta;
		double momentum_phi		= parameters.initial_energy_phi;
		
		spherical_to_cartesian<double>(
			momentum_rho,
			momentum_theta,
			momentum_phi,
			state_global.momentum_x,
			state_global.momentum_y,
			state_global.momentum_z);
	}
	else if (parameters.has_energy_cart)
	{
		state_global.momentum_x = energy_kinetic_to_momentum(particle.rest_mass, parameters.initial_energy_x / AU_ENERGY);
		state_global.momentum_y = energy_kinetic_to_momentum(particle.rest_mass, parameters.initial_energy_y / AU_ENERGY);
		state_global.momentum_z = energy_kinetic_to_momentum(particle.rest_mass, parameters.initial_energy_z / AU_ENERGY);
	}
}

void read_config_renders(Setting* field_renders_config, vector<FieldRender>& renders, vector<string>& headers, vector<string>& sources, Pulse& laser)
{
	try 
	{
		for (int i = 0; i < field_renders_config->getLength(); i++)
		{
			Setting& render_config = (*field_renders_config)[i];
			
			FieldRender render;
			
			render.id 		= string(render_config.getName());
			
			render.count	= 0;
			while (render_config.exists((bo::format("title_%u") % (render.count+1)).str()))
			{
				render.titles.push_back(render_config[(bo::format("title_%u") % (render.count+1)).str()]);
				render.count++;
			}
			
			if (render.count == 0)
			{
				printf("Unable to find '%s' parameter in render section '%s'\n", "title_1", render.id.c_str());
				exit(-1);
			}
			
			if (render.count > 8)
			{
				printf("Unfortunatly we can currently support up to 8 sub-renders. If you need more, contact the developer.\n");
				exit(-1);
			}
			
			
			if (render_config.exists("colors"))
			{
				for (unsigned short c = 0; c < render.count; c++)
					render.colors.push_back(render_config["colors"]);	
			}
			else
			{
				for (unsigned short c = 0; c < render.count; c++)
				{
					string color_id = (bo::format("color_%u") % (c+1)).str();
					if (render_config.exists(color_id))
						render.colors.push_back(render_config[color_id]);
					else
						missing_param(color_id);
				}
			}
			
			
			if (render_config.exists("enabled")) render.enabled = (bool)render_config["enabled"]; else missing_param("enabled");
			
			string plane;
			if (render_config.exists("plane"))	plane = (const char *) render_config["plane"]; else missing_param("plane");
			
			if (plane == "xy")
				render.plane = XY;
			if (plane == "xz")
				render.plane = XZ;
			if (plane == "yz")
				render.plane = YZ;
				
			string anchor = "origin";
			if (render_config.exists("anchor"))	anchor = (const char *) render_config["anchor"]; else missing_param("anchor");
			
			if (anchor == "origin")
				render.anchor = ORIGIN;
			if (anchor == "particle")
				render.anchor = PARTICLE;

			if (render_config.exists("axis_cut")) 			render.axis_cut 		= (double)render_config["axis_cut"] 			/ AU_LENGTH;	else render.axis_cut = 0;
			if (render_config.exists("space_resolution")) 	render.space_resolution	= (double)render_config["space_resolution"] / AU_LENGTH;	else missing_param("space_resolution");
			
			if (render_config.exists("space_size_x"))		render.space_size_x 	= (double)render_config["space_size_x"] 		/ AU_LENGTH; 	else missing_param("space_size_x");
			if (render_config.exists("space_size_y"))		render.space_size_y 	= (double)render_config["space_size_y"] 		/ AU_LENGTH; 	else missing_param("space_size_y");
			if (render_config.exists("space_size_z"))		render.space_size_z 	= (double)render_config["space_size_z"] 		/ AU_LENGTH; 	else missing_param("space_size_z");
			
			if (render_config.exists("time_resolution"))	render.time_resolution = (double)render_config["time_resolution"]	/ AU_TIME; 		else missing_param("time_resolution");
			if (render_config.exists("time_start"))			render.time_start 		= (double)render_config["time_start"]		/ AU_TIME; 		else missing_param("time_start");
			if (render_config.exists("time_end"))			render.time_end 		= (double)render_config["time_end"]			/ AU_TIME; 		else missing_param("time_end");
			
			if (render_config.exists("movie_length"))		render.movie_length 	= (double)render_config["movie_length"] 		/ AU_TIME; 		else missing_param("movie_length");

			if (!render_config.exists("formula")) missing_param("formula");
			
			
			headers.push_back((bo::format("extern \"C\" vector<double> func_field_render_%s(double t, double x, double y, double z);") % render.id).str());
			
			string s = (bo::format("vector<double> func_field_render_%s(double t, double x, double y, double z)\n") % render.id).str();
			s += "{\n";
			s += "    // Injecting default variables\n";
			
			
			s += (bo::format("    double dx 	__attribute__ ((unused)) = %.16E;\n") % (render.space_resolution / 1000 * AU_LENGTH)).str();
			s += (bo::format("    double dy 	__attribute__ ((unused)) = %.16E;\n") % (render.space_resolution / 1000 * AU_LENGTH)).str();
			s += (bo::format("    double dz 	__attribute__ ((unused)) = %.16E;\n") % (render.space_resolution / 1000 * AU_LENGTH)).str();
			
			s += (bo::format("    double size_x	__attribute__ ((unused)) = %.16E;\n") % (render.space_size_x * AU_LENGTH)).str();
			s += (bo::format("    double size_y	__attribute__ ((unused)) = %.16E;\n") % (render.space_size_y * AU_LENGTH)).str();
			s += (bo::format("    double size_z	__attribute__ ((unused)) = %.16E;\n") % (render.space_size_z * AU_LENGTH)).str();
			
			s += (bo::format("    double dt		__attribute__ ((unused)) = %.16E;\n") % (render.time_resolution  / 1000 * AU_TIME)).str();
			s += "    // completed\n\n";
			
			
			inject_laser_variables(s, laser, true);
			
			string formula = render_config["formula"];
			s += (bo::format("%s\n") % (formula)).str();
			s += "}\n";
				
			sources.push_back(s);
			


			renders.push_back(render);
		}
		
	}
	catch (ParseException& e)  
	{
		printf("Error while reading configuration file: %s\n", e.getFile());
		printf("Line %d: %s\n", e.getLine(), e.getError());
		exit(-1);
	}
}

double get_conversion_si(string type)
{
	if (type == "MASS") 				return AU_MASS;
	if (type == "LENGTH")				return AU_LENGTH;
	if (type == "CHARGE")				return AU_CHARGE;
	if (type == "ENERGY")				return AU_ENERGY;
	if (type == "TIME")					return AU_TIME;
	if (type == "FREQUENCY")			return 1.d;
	if (type == "PULSATION")			return 1.d;
	if (type == "SPEED")				return AU_SPEED;
	if (type == "FORCE")				return AU_FORCE;
	if (type == "ELECTRIC_FIELD")		return AU_ELECTRIC_FIELD;
	if (type == "ELECTRIC_POTENTIAL")	return AU_ELECTRIC_POTENTIAL;
	if (type == "MOMENTUM")				return AU_MOMENTUM;
	if (type == "MAGNETIC_FIELD")		return AU_MAGNETIC_FIELD;
	if (type == "ANGLE")				return 1.d;
	if (type == "PERCENTUAL")			return 1.d;
	if (type == "PURE_FLOAT")			return 1.d;
	if (type == "PURE_INT")				return 1.d;
	if (type == "IGNORE")				return 1.d;
	if (type == "IGNORE_START")			return 1.d;
	if (type == "IGNORE_END")			return 1.d;
	
	printf("Unable to find the conversion for unit: %s\n", type.c_str());
	exit(-1);
}

void read_config(
	fs::path& cfg_file,
	Simulation& simulation,
	Pulse& laser,
	Particle& particle,
	ParticleStateGlobal& particle_state,
	Laboratory& laboratory,
	vector<FieldRender>&      field_renders,
	vector<ResponseAnalysis>& response_analyses,
	vector<string>& headers,
	vector<string>& sources)
{
	Parameters parameters;
	
	Config* config = new Config;
	
	map<string, int>	laser_field_param_map_int;
	map<string, long>   laser_field_param_map_int64;
	map<string, double> laser_field_param_map_float;
	map<string, string>	laser_field_param_map_string;
	map<string, bool>	laser_field_param_map_boolean;
	


	// Read the file. If there is an error, report it and exit.
	try 
	{
		config->readFile(cfg_file.c_str());

		Setting&  config_simulation	 		= config->lookup("simulation");
		Setting&  config_laser 				= config->lookup("laser");
		Setting&  config_particle 			= config->lookup("particle");
		Setting&  config_laboratory 		= config->lookup("laboratory");
		Setting&  config_response_analyses	= config->lookup("response_analyses");

		config_laser.lookupValue			("func_fields",	parameters.func_fields)		|| missing_param("func_fields");
		
		static const bo::regex e("^func_param_([A-Za-z\\_0-9]+)$");
		for (int i = 0; i < config_laser.getLength(); i++)
		{
			Setting& setting = config_laser[i];
			
			bo::match_results<string::const_iterator> what;
			if (bo::regex_match(string(setting.getName()), what, e))
			{
				switch (setting.getType())
				{
					case Setting::TypeInt:
						laser_field_param_map_int[what[1]] = setting;
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
		

		config_laser.lookupValue			("timing_mode",  				parameters.timing_mode)				|| missing_param("timing_mode");
		config_laser.lookupValue			("timing_offset",  				parameters.timing_offset)			|| missing_param("timing_offset");

		config_particle.lookupValue			("rest_mass",  				parameters.rest_mass)					|| missing_param("rest_mass");
		config_particle.lookupValue			("charge",  				parameters.charge)						|| missing_param("charge");
		
		parameters.has_position_cart = config_particle.exists("initial_position_x") 		&& config_particle.exists("initial_position_y") 	&& config_particle.exists("initial_position_z");
		parameters.has_position_sphe = config_particle.exists("initial_position_rho") 		&& config_particle.exists("initial_position_theta")	&& config_particle.exists("initial_position_phi");
		parameters.has_momentum_cart = config_particle.exists("initial_momentum_x") 		&& config_particle.exists("initial_momentum_y") 	&& config_particle.exists("initial_momentum_z");
		parameters.has_momentum_sphe = config_particle.exists("initial_momentum_rho") 		&& config_particle.exists("initial_momentum_theta")	&& config_particle.exists("initial_momentum_phi");
		parameters.has_energy_cart   = config_particle.exists("initial_energy_x") 			&& config_particle.exists("initial_energy_y") 		&& config_particle.exists("initial_energy_z");
		parameters.has_energy_sphe   = config_particle.exists("initial_energy_rho") 		&& config_particle.exists("initial_energy_theta")	&& config_particle.exists("initial_energy_phi");

		if (parameters.has_position_cart)
		{
			config_particle.lookupValue	("initial_position_x",  		parameters.initial_position_x)	|| missing_param("initial_position_x");
			config_particle.lookupValue	("initial_position_y",  		parameters.initial_position_y)	|| missing_param("initial_position_y");
			config_particle.lookupValue	("initial_position_z",  		parameters.initial_position_z)	|| missing_param("initial_position_z");
		}
		
		if (parameters.has_position_sphe)
		{
			config_particle.lookupValue	("initial_position_rho",		parameters.initial_position_rho)	|| missing_param("initial_position_rho");
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
			config_particle.lookupValue	("initial_momentum_rho",		parameters.initial_momentum_rho)	|| missing_param("initial_momentum_rho");
			config_particle.lookupValue	("initial_momentum_theta",		parameters.initial_momentum_theta)	|| missing_param("initial_momentum_theta");
			config_particle.lookupValue	("initial_momentum_phi",  		parameters.initial_momentum_phi)	|| missing_param("initial_momentum_phi");
		}
		
		if (parameters.has_energy_cart)
		{
			config_particle.lookupValue	("initial_energy_x",  		parameters.initial_energy_x)	|| missing_param("initial_energy_x");
			config_particle.lookupValue	("initial_energy_y",  		parameters.initial_energy_y)	|| missing_param("initial_energy_y");
			config_particle.lookupValue	("initial_energy_z",  		parameters.initial_energy_z)	|| missing_param("initial_energy_z");
		}
		
		if (parameters.has_energy_sphe)
		{
			config_particle.lookupValue	("initial_energy_rho",		parameters.initial_energy_rho)	|| missing_param("initial_energy_rho");
			config_particle.lookupValue	("initial_energy_theta",	parameters.initial_energy_theta)|| missing_param("initial_energy_theta");
			config_particle.lookupValue	("initial_energy_phi",  	parameters.initial_energy_phi)	|| missing_param("initial_energy_phi");
		}



		config_simulation.lookupValue	("basename",  				parameters.basename)				|| missing_param("basename");
		config_simulation.lookupValue	("error_abs",  				parameters.error_abs)				|| missing_param("error_abs");
		config_simulation.lookupValue	("error_rel",  				parameters.error_rel)				|| missing_param("error_rel");
		config_simulation.lookupValue	("time_resolution_laser",	parameters.time_resolution_laser)	|| missing_param("time_resolution_laser");
		config_simulation.lookupValue	("time_resolution_free",	parameters.time_resolution_free)	|| missing_param("time_resolution_free");
		config_simulation.lookupValue	("duration",  				parameters.simulation_duration)		|| missing_param("duration (simulation)");
		config_simulation.lookupValue	("laser_influence_radius",  parameters.laser_influence_radius)	|| missing_param("laser_influence_radius");
		config_simulation.lookupValue	("labmap_max_size",  		parameters.labmap_max_size)			|| missing_param("labmap_max_size");
		config_simulation.lookupValue	("labmap_full",  			parameters.labmap_full)				|| missing_param("labmap_full");
		config_simulation.lookupValue	("func_commons",  			parameters.func_commons)			|| missing_param("func_commons");

		config_response_analyses.lookupValue ("enabled",  			parameters.response_analyses_enabled)	|| missing_param("response_analyses_enabled");
		
		static const bo::regex e_node("^node\\_([0-9]+)$");
		for (int i = 0; i < config_laboratory.getLength(); i++)
		{
			Setting& node_setting = config_laboratory[i];
			bo::match_results<string::const_iterator> what;
			string node_name = string(node_setting.getName());
			if (bo::regex_match(node_name, what, e_node))
			{
				Node node;
				
				node.id = stoi(what[1]);
				
				double position_x;
				double position_y;
				double position_z;
				
				node_setting.lookupValue("position_x", position_x) || missing_param("node_%d->position_x");
				node_setting.lookupValue("position_y", position_y) || missing_param("node_%d->position_y");
				node_setting.lookupValue("position_z", position_z) || missing_param("node_%d->position_z");
				
				position_x /= AU_LENGTH;
				position_y /= AU_LENGTH;
				position_z /= AU_LENGTH;
				
				node.position_x = position_x;
				node.position_y = position_y;
				node.position_z = position_z;
				
				node.axis = mat(3, 3, fill::eye);
				
				double theta;
				double phi;
				
				node_setting.lookupValue("rotation_theta", theta) || missing_param((bo::format("node_%d->rotation_theta") % node.id).str());
				node_setting.lookupValue("rotation_phi",   phi)   || missing_param((bo::format("node_%d->rotation_phi"  ) % node.id).str());
				
				rotate_spherical<double>(node.axis(0,0), node.axis(1,0), node.axis(2,0), theta, phi);
				rotate_spherical<double>(node.axis(0,1), node.axis(1,1), node.axis(2,1), theta, phi);
				rotate_spherical<double>(node.axis(0,2), node.axis(1,2), node.axis(2,2), theta, phi);
				
				// setting to zero all the versor components where abs(v) < 1E-15
				// We do this because these values are likely created by cos(π/2) != 0 and sin(0) != 0 functions
		   
				if (abs(node.axis(0,0)) < 1E-15)  node.axis(0,0) =  0; 
				if (abs(node.axis(1,0)) < 1E-15)  node.axis(1,0) =  0; 
				if (abs(node.axis(2,0)) < 1E-15)  node.axis(2,0) =  0; 
				if (abs(node.axis(0,1)) < 1E-15)  node.axis(0,1) =  0; 
				if (abs(node.axis(1,1)) < 1E-15)  node.axis(1,1) =  0; 
				if (abs(node.axis(2,1)) < 1E-15)  node.axis(2,1) =  0; 
				if (abs(node.axis(0,2)) < 1E-15)  node.axis(0,2) =  0; 
				if (abs(node.axis(1,2)) < 1E-15)  node.axis(1,2) =  0; 
				if (abs(node.axis(2,2)) < 1E-15)  node.axis(2,2) =  0; 
				
				laboratory.nodes.push_back(node);
			}
		}
		
		static const bo::regex e_resp1("^analysis\\_([0-9]+)$");
		static const bo::regex e_resp2("^\\s*analyze\\s+([a-zA-Z\\_\\s,]+)\\s+when\\s+([a-zA-Z\\_]+)\\s+([a-zA-Z\\_]+)\\s+([a-zA-Z\\_]+)\\s+changes\\s+(by\\s+([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\s*(\\%?))?(between\\s+([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\s*(\\%?)\\s+and\\s+([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\s*(\\%?))?\\s+in\\s+([0-9]+)\\s+steps\\s*$");
		
		for (int i = 0; i < config_response_analyses.getLength(); i++)
		{
			Setting& config_analysis = config_response_analyses[i];
			
			string analysis_name = config_analysis.getName();
			bo::match_results<string::const_iterator> what1;
			if (bo::regex_match(analysis_name, what1, e_resp1))
			{
				string analysis_expression = config_analysis;
				
				bo::match_results<string::const_iterator> what2;
				if (bo::regex_match(analysis_expression, what2, e_resp2))
				{
					ResponseAnalysis response_analysis;
					response_analysis.id = stoi(what1[1]);
					response_analysis.enabled = parameters.response_analyses_enabled;
					
					string last_object = "";
					
					string tokens_str = what2[1];
					vector<string> tokens;
					
					bo::split(tokens, tokens_str, bo::is_any_of(","));
					for (string token: tokens)
					{
						vector<string> parts;
						// removing duplicated spaces and cleaning
						token = bo::regex_replace(token, bo::regex("[[:blank:]]{2,}"), " ");
						token = bo::regex_replace(token, bo::regex("^[[:blank:]]"),    "");
						token = bo::regex_replace(token, bo::regex("[[:blank:]]$"),    "");
						
						bo::split(parts, token, bo::is_any_of(" "));
						
						if (parts.size() == 2)
						{
							last_object = parts[0];
							response_analysis.object_out.push_back(parts[0]);
							response_analysis.attribute_out.push_back(parts[1]);
						}
						else if (parts.size() == 1)
						{
							if (last_object != "")
							{
								response_analysis.object_out.push_back(last_object);
								response_analysis.attribute_out.push_back(parts[0]);
							}
							else
							{
								printf("ERROR - Error parsing token '%s'. The first token must be <object> <attribute> while the next tokens can omit the <object> part.\n", token.c_str());
								exit(-1);
								return;	
							}
						}
						else
						{
							printf("ERROR - Error parsing token '%s'. Expected format <object> <attribute>\n", token.c_str());
							exit(-1);
							return;	
						}
					}
					
					response_analysis.object_in		= what2[2];
					response_analysis.attribute_in	= what2[3];
					
					if (what2[4] == "linearly")
						response_analysis.change_type		= LINEAR;
					else if (what2[4] == "randomly")
						response_analysis.change_type		= RANDOM;
					else
					{
						printf("ERROR - Wrong value for '%s' expression. Allowed mode are: 'linerly' or 'randomly' but '%s' was found.\n", config_analysis.getName(), string(what2[4]).c_str());
						exit(-1);
						return;	
					}
					
					
					if (what2[5] != "" && what2[9] == "")
					{
						if (what2[8] == "%")
						{
							response_analysis.value_type    = PERCENTUAL;
							response_analysis.change_from	= -stod(what2[6]) / 100.d;
							response_analysis.change_to		= +stod(what2[6]) / 100.d;
						}
						else
						{
							response_analysis.value_type    = VALUE_RELATIVE;
							response_analysis.change_from	= -stod(what2[6]) / get_conversion_si_value(response_analysis.object_in, response_analysis.attribute_in);
							response_analysis.change_to		= +stod(what2[6]) / get_conversion_si_value(response_analysis.object_in, response_analysis.attribute_in);
						}
					}
					else if (what2[5] == "" && what2[9] != "")
					{
						if (what2[12] == "%" && what2[15] == "%")
						{
							response_analysis.value_type    = PERCENTUAL;
							response_analysis.change_from	= stod(what2[10]) / 100.d;
							response_analysis.change_to		= stod(what2[13]) / 100.d;
						}
						else if (what2[12] == "" && what2[15] == "")
						{
							response_analysis.value_type    = VALUE_ABSOLUTE;
							response_analysis.change_from	= stod(what2[10]) / get_conversion_si_value(response_analysis.object_in, response_analysis.attribute_in);
							response_analysis.change_to		= stod(what2[13]) / get_conversion_si_value(response_analysis.object_in, response_analysis.attribute_in);
						}
						else
						{
							printf("ERROR - Error in %s syntax: when using between, both elements must be percentual or absolute values.\n", config_analysis.getName());
							exit(-1);
							return;
						}
					}
					else
					{
						printf("ERROR - Error in %s syntax: we must have one 'by' clause or 'between' clause.\n", config_analysis.getName());
						exit(-1);
						return;
					}
					
					
					response_analysis.steps	= stoi(what2[16]);
					response_analyses.push_back(response_analysis);
				}
				else
				{
					printf("ERROR - Wrong format for '%s' parameter.\n", config_analysis.getName());
					printf("Allowed format are:\n");
					printf("  analyze particle <attribute> when <object> <attribute> linearly changes by <value> in <n> steps\n");
					printf("  analyze particle <attribute> when <object> <attribute> randomly changes by <value> in <n> steps\n");
					printf("  analyze particle <attribute> when <object> <attribute> linearly changes between <value_1> and <value_2> in <n> steps\n");
					printf("  analyze particle <attribute> when <object> <attribute> randomly changes between <value_1> and <value_2> in <n> steps\n");
					printf("Provided format was:\n");
					printf("  %s\n", analysis_expression.c_str());
					exit(-1);
					return;
				}
			}
			else if (string(config_analysis.getName()) != "enabled")
			{
				printf("ERROR - Wrong '%s' parameter name. Allowed format is: analysis_<1-9999>\n", config_analysis.getName());
				exit(-1);
				return;
			}
			
		}
		
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
	
	if (simulation.time_resolution_laser >= simulation.time_resolution_free)
	{
		printf("ERROR - 'time_resolution_laser' must be lower than 'time_resolution_free'\n");
		exit(-1);
		return;
	}
	
	simulation.laser_influence_radius	= parameters.laser_influence_radius	/ AU_LENGTH;
	simulation.labmap_max_size			= parameters.labmap_max_size;
	simulation.labmap_full				= parameters.labmap_full;
	simulation.duration					= parameters.simulation_duration 	/ AU_TIME;

	
	laser.params.params_int64		= laser_field_param_map_int64;
	laser.params.params_float		= laser_field_param_map_float;
	laser.params.params_string		= laser_field_param_map_string;
	laser.params.params_boolean		= laser_field_param_map_boolean;
	
	if (parameters.timing_mode == "enter")
		laser.timing_mode = ENTER;
	else if (parameters.timing_mode == "nearest")
		laser.timing_mode = NEAREST;
	else if (parameters.timing_mode == "exit")
		laser.timing_mode = EXIT;
	else
	{
		printf("ERROR - timing_mode has unknown values. Allowed values are: enter, nearest, exit'\n");
		exit(-1);
		return;
	}
	
	laser.timing_offset = parameters.timing_offset / AU_TIME;
	
	
	
	// Loading the common functins.
	sources.push_back(parameters.func_commons);
	
	
	
	// Loading field function
	
	headers.push_back("extern \"C\" Field field(double t, double x, double y, double z, const PulseParams& params);");
	
	
	string s1 = "// This function is only for internal use\n";
	s1 += (bo::format("Field field(double t, double x, double y, double z)\n")).str();
	s1 += "{\n";
	
	inject_laser_variables(s1, laser, false);
	
	s1 += parameters.func_fields + "\n";
	s1 += "}\n";
	
	sources.push_back(s1);
		
	string s2 = (bo::format("Field field(double t, double x, double y, double z, const PulseParams& params)\n")).str();
	s2 += "{\n";
	
	link_laser_variables(s2, laser, false);
	
	s2 += parameters.func_fields + "\n";
	s2 += "}\n";
	
	sources.push_back(s2);

	
	

	particle.rest_mass 					= parameters.rest_mass 				/ AU_MASS;
	particle.charge 					= parameters.charge    				/ AU_CHARGE;
	
	Setting& field_renders_setting = config->lookup("field_renders");
	read_config_renders(&field_renders_setting, field_renders, headers, sources, laser);

	delete config;
	
	//init_nodes(accellerator);
	init_position_and_momentum(parameters, particle, laboratory, particle_state);
	

}
