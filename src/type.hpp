#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <LuaState.h>
#include <armadillo>

#ifndef CIRCLESIM_TYPE
#define CIRCLESIM_TYPE

using namespace arma;
using namespace std;

namespace fs = boost::filesystem;
namespace ba = boost::algorithm;
namespace bo = boost;


typedef enum {FREE, LASER}				 		RangeMode;
typedef enum {TURN_ON, CONSTANT, TURN_OFF} 		TimingMode;
typedef enum {MODE_P, MODE_T_FW, MODE_T_BW} 	NodeAxisMode;

#define pow2(a) ((a) * (a)) 
#define pow3(a) ((a) * (a) * (a)) 

#define C0 	137.036	// a.u


// Fondamental units
																						// SI		Symb	Description
																						// ----------------------------------------
#define AU_MASS	   				 9.109382616E-31										// Kg  		mₑ		electron mass
#define AU_LENGTH  				 5.29177210818E-11										// m   		a₀		Bohr radius
#define AU_CHARGE 				-1.602765314E-19										// C   		e		electron charge
#define AU_ANG_MOMENTUM 		 1.0545716818E-34										// J·s 		ℏ		reduced Planck constant
#define AU_ENERGY				 4.3597441775E-18										// J   		Eₕ		Hartree energy

// Derived units
#define AU_WAVE_VECTOR    		(1 / AU_LENGTH)	 										// m⁻¹  	1/a₀
#define AU_TIME    				(AU_ANG_MOMENTUM / AU_ENERGY)	 						// s  		ℏ/Eₕ
#define AU_SPEED   				(AU_LENGTH * AU_ENERGY / AU_ANG_MOMENTUM)				// m·s⁻¹	a₀·Eₕ/ℏ
#define AU_FORCE   				(AU_ENERGY / AU_LENGTH)	 								// N		Eₕ/a₀
#define AU_ELECTRIC_FIELD		(AU_ENERGY / AU_CHARGE / AU_LENGTH) 					// V·m⁻¹	Eₕ/(ea₀)
#define AU_ELECTRIC_POTENTIAL	(AU_ENERGY / AU_CHARGE) 								// V		Eₕ/e
#define AU_MOMENTUM				(AU_SPEED  * AU_MASS)									// m·s⁻¹·Kg	a₀·ℏ·mₑ/Eₕ
#define AU_FREQUENCY			(AU_ENERGY / AU_ANG_MOMENTUM)							// s⁻¹		Eₕ/ℏ
#define AU_MAGNETIC_FIELD		(AU_ANG_MOMENTUM / AU_CHARGE / AU_LENGTH / AU_LENGTH) 	// T		ℏ/(ea₀²)


typedef struct ParametersStruct
{
	string 			basename;
	double 		 	simulation_duration;

	double 			time_resolution_laser;
	double 			time_resolution_free;
	double			laser_influence_radius;

	double 			error_abs;
	double 			error_rel;

	double pulse_duration;	

	string func_commons;
	string func_fields;

	double rest_mass;
	double charge;

	unsigned int initial_reference_node;

	bool has_position_cart;
	bool has_position_sphe;
	bool has_momentum_cart;
	bool has_momentum_sphe;
	
	double initial_position_x;
	double initial_position_y;
	double initial_position_z;
	double initial_position_module;
	double initial_position_theta;
	double initial_position_phi;
	
	double initial_momentum_x;
	double initial_momentum_y;
	double initial_momentum_z;
	double initial_momentum_module;
	double initial_momentum_theta;
	double initial_momentum_phi;

	unsigned int 	nodes;
	double 			radius;
	
	string			node_axis_mode;
	bool			node_axis_rotate_theta;
	bool			node_axis_rotate_phi;
	  
	string			timing_mode;
	double          timing_value;
	
	bool field_map_enable_xy; 
	bool field_map_enable_xz;
	bool field_map_enable_yz;

	vector<string> field_map_xy_skip;
	vector<string> field_map_xz_skip;
	vector<string> field_map_yz_skip;
	
	double field_map_resolution_t;
	double field_map_resolution_x;
	double field_map_resolution_y;
	double field_map_resolution_z;
	
	double field_map_size_x;
	double field_map_size_y;
	double field_map_size_z;
	
	double field_map_movie_length;
} Parameters;

typedef struct SimulationStruct
{
	string 			basename;
	double		 	duration;
	
	double		 	time_resolution_laser;
	double 			time_resolution_free;
	
	double			laser_influence_radius;
	
	double 			error_abs;
	double 			error_rel;
	
} Simulation;

typedef struct PulseStruct
{
	double	duration;		// pulse duration
	int		func_fields;	// coockie of LUA function
} Pulse;

typedef struct FieldEBStruct
{
	double e_x;
	double e_y;
	double e_z;

	double b_x;
	double b_y;
	double b_z;
} FieldEB;

typedef struct FieldEBLimitsStruct
{
	double e_x_min;
	double e_y_min;
	double e_z_min;
	
	double e_x_max;
	double e_y_max;
	double e_z_max;
	
	double b_x_min;
	double b_y_min;
	double b_z_min;
	
	double b_x_max;
	double b_y_max;
	double b_z_max;
} FieldEBLimits;

typedef struct ParticleStruct
{
	double rest_mass;
	double charge;
} Particle;

typedef struct ParticleStateStruct
{	
	
	/*
	 * The position measure unit is the A.U. and is equal to 5.29177210818E-11 meters
	 * 
	 * Our mantiss range is this:
	 * We must to simulate the trajectory an accellerator with radius up to 10 Km (~1E5 meters) with a precision equals to electron radius, like 2.8179403267E-15 meters (~1E-15). 
	 * Our mantiss must be able to contain a number between 0 and 10E20: ln(1E20)/ln(2) =  66.4385618977 → 64 bit (we round to a power of 2 because it is nicer)
	 * 
	 * Our exponent range is this:
	 * Our unit measure is A.U. (~1E-11). We must be able to fall down to electron radius (~E-15) and to grow up to accellerator radius (~1E5).
	 * This means that our exponent will have as minimum value -4 and as upper value 16. Because the size of exponent is common we have to choose the bigger (in module) value and calculate the bit needed for its representation.
	 * ln(|16|)/ln(2) = 4 bits
	 * 
	 * Our floating point for a vector position must have at least:
	 * 
	 * [64 bit mantiss] + [4 bit exponent]
	 * 
	 * C++ has a type named long double structured so:
	 * [S][  Exponent  ][I][  Fraction  ]
	 *  1       15       1       63
	 * 
	 * This fit exaxtly our needs so we can use it. Every position component will expressed by a 10 byte variable full supported by processor FPU (in x386 architecture)
	 */
	
	long double position_x;
	long double position_y;
	long double position_z;
	
	double momentum_x;
	double momentum_y;
	double momentum_z;
} ParticleState;

typedef struct NodeStruct
{
	unsigned int	id;
	long double 	position_x;
	long double 	position_y;
	long double 	position_z;
	mat				axis; 				// Axis rotation 3x3 matrix
} Node;

typedef struct AccelleratorStruct
{  
	vector<Node> 	nodes;
	double 			radius;

	NodeAxisMode	node_axis_mode;
	bool			node_axis_rotate_theta;
	bool			node_axis_rotate_phi;

	TimingMode 		timing_mode;
	double          timing_value;
} Accellerator;



typedef struct OutputSettingStruct
{
	bool field_map_enable_xy; 
	bool field_map_enable_xz;
	bool field_map_enable_yz;
	
	vector<bo::regex> field_map_xy_skip;
	vector<bo::regex> field_map_xz_skip;
	vector<bo::regex> field_map_yz_skip;

	double field_map_resolution_t;
	double field_map_resolution_x;
	double field_map_resolution_y;
	double field_map_resolution_z;
	
	double field_map_size_x;
	double field_map_size_y;
	double field_map_size_z;
	
	double field_map_movie_length;

} OutputSetting;

#endif
