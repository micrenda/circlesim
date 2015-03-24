#include <libconfig.h++>
#include <math.h>
#include <LuaState.h>
#include <boost/regex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>
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


//void init_nodes(Accellerator& accellerator)
//{
	//unsigned int n = 0;
	//for (Node& node: accellerator.nodes)
	//{
		//node.position_x = accellerator.radius * sin(2 * M_PI * n / accellerator.nodes.size());
		//node.position_y = accellerator.radius * cos(2 * M_PI * n / accellerator.nodes.size());
		//node.position_z = 0;
		
		
		//double theta	= 0; 
		//double phi		= 0;
		
		//if (accellerator.node_axis_rotate_theta)
			//theta = +2 * M_PI * n / accellerator.nodes.size();
		//if (accellerator.node_axis_rotate_phi)
			//theta = -2 * M_PI * n / accellerator.nodes.size();
		
		//mat versors = zeros<mat>(3,3);
		
		//switch (accellerator.node_axis_mode)
		//{
			//case MODE_P:
				//versors(0,0) =  1; 
				//versors(1,0) =  0; 
				//versors(2,0) =  0;
				
				//versors(0,1) =  0; 
				//versors(1,1) =  1; 
				//versors(2,1) =  0;
				
				//versors(0,2) =  0; 
				//versors(1,2) =  0; 
				//versors(2,2) =  1;
			//break;
			
			
			//case MODE_T_FW:
				//versors(0,0) =  0; 
				//versors(1,0) =  1; 
				//versors(2,0) =  0;
				
				//versors(0,1) =  0; 
				//versors(1,1) =  0; 
				//versors(2,1) =  1;
				
				//versors(0,2) =  1; 
				//versors(1,2) =  0; 
				//versors(2,2) =  0;
			//break;
			
			//case MODE_T_BW:
				//versors(0,0) =  0; 
				//versors(1,0) = -1; 
				//versors(2,0) =  0;
				
				//versors(0,1) =  0; 
				//versors(1,1) =  0; 
				//versors(2,1) =  1;
				
				//versors(0,2) = -1; 
				//versors(1,2) =  0; 
				//versors(2,2) =  0;
			//break;

		//}
		
		//rotate_spherical(versors(0,0), versors(1,0), versors(2,0), theta, phi);
		//rotate_spherical(versors(0,1), versors(1,1), versors(2,1), theta, phi);
		//rotate_spherical(versors(0,2), versors(1,2), versors(2,2), theta, phi);
		
		//// setting to zero all the versor components where abs(v) < 1E-15
		//// We do this because these values are likely created by cos(π/2) != 0 and sin(0) != 0functions and
   
		//if (abs(versors(0,0)) < 1E-15)  versors(0,0) =  0; 
		//if (abs(versors(1,0)) < 1E-15)  versors(1,0) =  0; 
		//if (abs(versors(2,0)) < 1E-15)  versors(2,0) =  0; 
		//if (abs(versors(0,1)) < 1E-15)  versors(0,1) =  0; 
		//if (abs(versors(1,1)) < 1E-15)  versors(1,1) =  0; 
		//if (abs(versors(2,1)) < 1E-15)  versors(2,1) =  0; 
		//if (abs(versors(0,2)) < 1E-15)  versors(0,2) =  0; 
		//if (abs(versors(1,2)) < 1E-15)  versors(1,2) =  0; 
		//if (abs(versors(2,2)) < 1E-15)  versors(2,2) =  0; 
			
		
		//node.axis = versors;
		
//#ifdef DEBUG_NODE_VERSORS
		//printf("Node %d\n", node.id);
		//node.axis.print("local versors:");
		//printf("\n\n");
//#endif

		//n++;
	//}
//}


void init_position_and_momentum(Parameters& parameters, Laboratory& laboratory, ParticleStateGlobal& state_global)
{
	// Setting the position of the particle relative to selected node
	
	if (parameters.initial_reference_node < 0 || parameters.initial_reference_node >= laboratory.nodes.size())
	{
		wrong_param("initial_reference_node", "Invalid node index\n");
	}
		
	Node& first_node = laboratory.nodes[parameters.initial_reference_node];
	
	
	// Initializing POSITION 
	ParticleStateLocal state_local;
	
	
	if (parameters.has_position_sphe && parameters.has_position_cart)
	{
		wrong_param("initial_position", "You must to specify one of spherical or cartesian position. You can not specify both.\n");
	}
	else if (parameters.has_position_sphe)
	{
		spherical_to_cartesian(
			parameters.initial_position_rho / AU_LENGTH,
			parameters.initial_position_theta,
			parameters.initial_position_phi,
			state_local.position_x,
			state_local.position_y,
			state_local.position_z);
	}
	else if (parameters.has_position_cart)
	{
		state_local.position_x	= parameters.initial_position_x / AU_LENGTH;
		state_local.position_y	= parameters.initial_position_y / AU_LENGTH;
		state_local.position_z	= parameters.initial_position_z / AU_LENGTH;
	}
	else
	{
		wrong_param("initial_position", "You must to specify an initial position (cartesian or spherical).\n");
	}
	
	
	// Initializing MOMENTUM 
	if (parameters.has_momentum_sphe && parameters.has_momentum_cart)
	{
		wrong_param("initial_momentum", "You must to specify one of spherical or cartesian momentum. You can not specify both.\n");
	}
	else if (parameters.has_momentum_sphe)
	{
		spherical_to_cartesian<double>(
			parameters.initial_momentum_rho / AU_MOMENTUM,
			parameters.initial_momentum_theta,
			parameters.initial_momentum_phi,
			state_local.momentum_x,
			state_local.momentum_y,
			state_local.momentum_z);
	}
	else if (parameters.has_momentum_cart)
	{
		state_local.momentum_x	= parameters.initial_momentum_x / AU_MOMENTUM;
		state_local.momentum_y	= parameters.initial_momentum_y / AU_MOMENTUM;
		state_local.momentum_z	= parameters.initial_momentum_z / AU_MOMENTUM;
	}
	else
	{
		wrong_param("initial_momentum", "You must to specify an initial momentum (cartesian or spherical).\n");
	}	
		
	
	// Converting to global state
	state_local_to_global(state_global, state_local, first_node);
}

void read_config_renders(Setting* field_renders_config, set<FieldRender>& renders, lua::State* lua_state)
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
			
			render.func_formula_name = (bo::format("func_field_render_%s") % render.id).str();
			string s = (bo::format("function %s(D, t, x, y, z)\n") % render.func_formula_name).str();
			s += "    -- Injecting default variables\n";
			s += (bo::format("    dx = %.16E\n") % (render.space_resolution / 1000 * AU_LENGTH)).str();
			s += (bo::format("    dy = %.16E\n") % (render.space_resolution / 1000 * AU_LENGTH)).str();
			s += (bo::format("    dz = %.16E\n") % (render.space_resolution / 1000 * AU_LENGTH)).str();
			
			s += (bo::format("    size_x   = %.16E\n") % (render.space_size_x * AU_LENGTH)).str();
			s += (bo::format("    size_y   = %.16E\n") % (render.space_size_y * AU_LENGTH)).str();
			s += (bo::format("    size_z   = %.16E\n") % (render.space_size_z * AU_LENGTH)).str();
			
			s += (bo::format("    dt = %.16E\n") % (render.time_resolution  / 1000 * AU_TIME)).str();
			s += "    -- completed\n\n";
			
			string formula = render_config["formula"];
			s += (bo::format("%s\n") % (formula)).str();
			s += "end\n";
			
			try
			{
				lua_state->doString(s.c_str());
			}
			catch (lua::LoadError& e)
			{
				check_lua_error(e, render.func_formula_name, s);
			}
			
			printf((bo::format("Render field %s LUA function:\n") % render.id).str().c_str());
			printf("--------------------------------------------------------\n");
			printf("%s\n", s.c_str());
			printf("--------------------------------------------------------\n");


			renders.insert(render);
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
	set<FieldRender>&      field_renders,
	set<ResponseAnalysis>& response_analyses,
	lua::State* lua_state)
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
		

		config_particle.lookupValue			("rest_mass",  				parameters.rest_mass)					|| missing_param("rest_mass");
		config_particle.lookupValue			("charge",  				parameters.charge)						|| missing_param("charge");
		
		config_particle.lookupValue			("initial_reference_node",  parameters.initial_reference_node)	|| missing_param("initial_reference_node");

		parameters.has_position_cart = config_particle.exists("initial_position_x") 		&& config_particle.exists("initial_position_y") 	&& config_particle.exists("initial_position_z");
		parameters.has_position_sphe = config_particle.exists("initial_position_rho") 		&& config_particle.exists("initial_position_theta")	&& config_particle.exists("initial_position_phi");
		parameters.has_momentum_cart = config_particle.exists("initial_momentum_x") 		&& config_particle.exists("initial_momentum_y") 	&& config_particle.exists("initial_momentum_z");
		parameters.has_momentum_sphe = config_particle.exists("initial_momentum_rho") 		&& config_particle.exists("initial_momentum_theta")	&& config_particle.exists("initial_momentum_phi");

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


		config_simulation.lookupValue		("basename",  				parameters.basename)				|| missing_param("basename");
		config_simulation.lookupValue		("error_abs",  				parameters.error_abs)				|| missing_param("error_abs");
		config_simulation.lookupValue		("error_rel",  				parameters.error_rel)				|| missing_param("error_rel");
		config_simulation.lookupValue		("time_resolution_laser",	parameters.time_resolution_laser)	|| missing_param("time_resolution_laser");
		config_simulation.lookupValue		("time_resolution_free",	parameters.time_resolution_free)	|| missing_param("time_resolution_free");
		config_simulation.lookupValue		("duration",  				parameters.simulation_duration)		|| missing_param("duration (simulation)");
		config_simulation.lookupValue		("laser_influence_radius",  parameters.laser_influence_radius)	|| missing_param("laser_influence_radius");
		config_simulation.lookupValue		("func_commons",  			parameters.func_commons)			|| missing_param("func_commons");


		
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
		static const bo::regex e_resp2("^\\s*analyze\\s+([a-zA-Z\\_]+)\\s+([a-zA-Z\\_]+)\\s+when\\s+([a-zA-Z\\_]+)\\s+([a-zA-Z\\_]+)\\s+([a-zA-Z\\_]+)\\s+changes\\s+by\\s+([0-9\\.]+)\\%\\s+in\\s+([0-9]+)\\s+steps\\s*$");
		
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
					
					response_analysis.object_out	= what2[1];
					response_analysis.attribute_out	= what2[2];
					response_analysis.object_in		= what2[3];
					response_analysis.attribute_in	= what2[4];
					response_analysis.change_range	= stod(what2[5]) / 100.d;
					response_analysis.change_steps	= stoi(what2[6]);
					
					if (what2[6] == "linearly")
						response_analysis.change_mode		= LINEAR;
					else if (what2[6] == "randomly")
						response_analysis.change_mode		= RANDOM;
					else
					{
						printf("ERROR - Wrong value for '%s' expression. Allowed mode are: linerly or randomly.\n", config_analysis.getName());
						exit(-1);
						return;	
					}
					
					response_analyses.insert(response_analysis);
				}
				else
				{
					printf("ERROR - Wrong format for '%s' parameter.\n", config_analysis.getName());
					printf("Allowed format are:\n");
					printf("  analyze particle <attribute> when <object> <attribute> linearly changes by <p>%% in <n> steps\n");
					printf("  analyze particle <attribute> when <object> <attribute> randomly changes by <p>%% in <n> steps\n");
					
					exit(-1);
					return;
				}
			}
			else
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
	simulation.laser_influence_radius	= parameters.laser_influence_radius	/ AU_LENGTH;
	simulation.duration					= parameters.simulation_duration 	/ AU_TIME;
	
	laser.duration						= parameters.pulse_duration 		/ AU_TIME;
	
	
	laser.params_int64		= laser_field_param_map_int64;
	laser.params_float		= laser_field_param_map_float;
	laser.params_string		= laser_field_param_map_string;
	laser.params_boolean	= laser_field_param_map_boolean;
	
	
	// Loading the common functins. Loading and evaluating so remain availabe to the other LUA scripts
	lua_state->doString(parameters.func_commons);
	//check_lua_error(lua_state, err, "func_commons", parameters.func_commons);
	// Loading other lua scripts
	
	string s = "function func_fields(D, t, x, y, z)\n";
	s += "    -- Injecting external variables\n";
	for (map<string,int>::iterator	entry = laser.params_int.begin(); entry != laser.params_int.end(); ++entry) 
		s += (bo::format("    % -12s\t= %d\n") 		% entry->first % entry->second).str();
	for (map<string,long>::iterator	entry = laser.params_int64.begin(); entry != laser.params_int64.end(); ++entry) 
		s += (bo::format("    % -12s\t= %l\n") 		% entry->first % entry->second).str();
	for (map<string,double>::iterator	entry = laser.params_float.begin(); entry != laser.params_float.end(); ++entry) 
		s += (bo::format("    % -12s\t= %.16E\n") 	% entry->first % entry->second).str();
	for (map<string,string>::iterator	entry = laser.params_string.begin(); entry != laser.params_string.end(); ++entry)
	{
		string value = entry->second;
		bo::replace_all(value, "\"", "\\\"");
		s += (bo::format("    % -12s\t= %s\n") 		% entry->first % value).str();
	}
	for (map<string,bool>::iterator	entry = laser.params_boolean.begin(); entry != laser.params_boolean.end(); ++entry) 
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
	
	Setting& field_renders_setting = config->lookup("field_renders");
	read_config_renders(&field_renders_setting, field_renders, lua_state);

	delete config;
	
	//init_nodes(accellerator);
	init_position_and_momentum(parameters, laboratory, particle_state);
}
