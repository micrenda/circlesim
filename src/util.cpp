#include <math.h>
#include <armadillo>
#include "type.hpp"
#include "util.hpp"


using namespace arma;


void unique_spherical_angles(double& theta, double& phi)
{
	// Fixing θ
	theta = fmod(theta, 2*M_PI);	// Fixing multple of 2π
	
	if (theta < 0)		// Fixing negative values
	{	
		theta = -theta;
		phi += M_PI;
	}
	
	if (theta > M_PI)	// Fixing θ > π
	{
		theta = 2*M_PI - theta;
		phi += M_PI;
	}
	
	// Fixing φ
	phi = fmod(phi, 2*M_PI);	// Fixing multple of 2π
		
	if (phi < 0)		// Fixing negative values
	{	
		phi = 2*M_PI - phi;
	}
	
}

// Rules applied:
// r ∈ [0, ∞)
// θ ∈ [0, π]
// φ ∈ [0,2π)
void unique_spherical_all(double& module, double& theta, double& phi)
{
	if (module < 0)
	{
		module = -module;
		theta += 2 * M_PI;
	}
	
	 unique_spherical_angles(theta, phi);
}

//template<typename T>
//T vector_module(T x1, T x2, T x3)
//{
	//return hypot(hypot(x1, x2), x3);
//}

double vector_module(double x1, double x2, double x3)
{
	return hypot(hypot(x1, x2), x3);
}

long double vector_module(long double x1, long double x2, long double x3)
{
	return hypot(hypot(x1, x2), x3);
}

int vector_module(int x1, int x2, int x3)
{
	return hypot(hypot(x1, x2), x3);
}

long vector_module(long x1, long x2, long x3)
{
	return hypot(hypot(x1, x2), x3);
}


void spherical_to_cartesian(double rho, double theta, double phi, double& x, double& y, double& z)
{
	x = rho * sin(theta) * cos(phi);
	y = rho * sin(theta) * sin(phi);
	z = rho * cos(theta);
}

void cartesian_to_spherical(double x, double y, double z, double& theta, double& phi)
{
	theta	= acos(z / sqrt(x*x + y*y + z*z));
	
	if (y == 0 && x == 0)
		phi		= 0; // We cover the case when we select the nord or south pole
	else
		phi		= atan(y / x);
	
	unique_spherical_angles(theta, phi);
}

double energy_to_momentum(double rest_mass, double energy)
{
	//	We use these formule to find the relativistic momentum:
	// 	p = mᵣv		E=mᵣc²		where mᵣ = m₀ + mᵣ
	//	We define these two units:
	//	β = v/c  			γ = 1 / √(1-v²/c²)
	//	Combining the last two definitions we get
	//  β = √(1-1/γ²)		γ = 1 / √(1-β²) 
	// 
	//	Using the formulas above we get:
	//	p = m₀γv = m₀γβ/c = m₀γ√(1-1/γ²)/c = m₀√(γ²-1)/c
	//	Εxtracting γ from energy:
	//  E = m₀γc²  → γ = E/(c²m₀)
	//	So at the end we can get:
	//  p = m₀√(γ²-1)/c = m₀√(E²/(c⁴m₀²)-1)/c
	return rest_mass * sqrt(energy*energy / (C0*C0*C0*C0 * rest_mass*rest_mass) - 1) / C0;
}




// Spherical rotation. TODO: write documentation
void rotate_spherical(double& x, double& y, double& z, double theta, double phi)
{
	double buffer_rho;
	double buffer_theta;
	double buffer_phi;
	
	cartesian_to_spherical(x, y, z, buffer_theta, buffer_phi);
	
	buffer_theta 	+= theta;
	buffer_phi		+= phi;
	
	unique_spherical_angles(buffer_theta, buffer_phi);
	
	buffer_rho = vector_module(x, y, z);
	
	spherical_to_cartesian(buffer_rho, buffer_theta, buffer_phi, x, y, z);
}


 /**
  * Euler rotation. It will update the variables x,y,z with new values after its rotation
  * 
  * α	Rotation around z-axis
  * β	Rotation around y-axis
  * γ	Rotation around x-axis
  * 
  * Rotate using these rotation matrixes:
  * 
  * Rx = 	|     1       0	      0  |
  *   		|     0   cos(γ) -sin(γ) |
  *			|     0   sin(γ)  cos(γ) |
  * 		
  * Ry = 	| cos(β)      0	  sin(β) |
  * 		|     0       1       0  |
  * 		|-sin(β)      0   cos(β) |
  * 
  * Rz = 	| cos(α) -sin(α)      0  |
  * 		| sin(α)  cos(α)      0  |
  * 		|     0       0       1  |
  */
void rotate_euler(double& x, double& y, double& z, double alpha, double beta, double gamma)
{
	vec v = zeros<vec>(3);
	
	v(0) = x;
	v(1) = y;
	v(2) = z;
	
	if (alpha != 0.0)
	{
		mat rx = zeros<mat>(3,3);
		
		rx(0, 0) = 1.0;
		rx(0, 1) = 0.0;
		rx(0, 2) = 0.0;
		rx(1, 0) = 0.0;
		rx(1, 1) =  cos(gamma);
		rx(1, 2) = -sin(gamma);
		rx(2, 0) = 0.0;
		rx(2, 1) =  sin(gamma);
		rx(2, 2) =  cos(gamma);
	
		v = rx * v;
	}
	
	if (beta  != 0.0)
	{
		mat ry = zeros<mat>(3,3);
			
		ry(0, 0) =  cos(beta);
		ry(0, 1) = 0.0;
		ry(0, 2) =  sin(beta);
		ry(1, 0) = 0.0;
		ry(1, 1) = 1.0;
		ry(1, 2) = 0.0;
		ry(2, 0) = -sin(beta);
		ry(2, 1) = 0.0;
		ry(2, 2) =  cos(beta);
	
		v = ry * v;
	}
	
	if (gamma  != 0.0)
	{
		mat rz = zeros<mat>(3,3);
	
		rz(0, 0) =  cos(alpha);
		rz(0, 1) = -sin(alpha);
		rz(0, 2) = 0.0;
		rz(1, 0) =  sin(alpha);
		rz(1, 1) =  cos(alpha);
		rz(1, 2) = 0.0;
		rz(2, 0) = 0.0;
		rz(2, 1) = 0.0;
		rz(2, 2) = 1.0;
		
		v = rz * v;
	}
	
	x = v(0);
	y = v(1);
	z = v(2);
}






void state_global_to_local(
	ParticleState& state, 
	Node& node,
	double& local_position_x,
	double& local_position_y,
	double& local_position_z,
	double& local_momentum_x,
	double& local_momentum_y,
	double& local_momentum_z)
{
	// Position
	
	vec tr_pos = zeros<vec>(3);
	tr_pos(0) = state.position_x - node.position_x;
	tr_pos(1) = state.position_y - node.position_y;
	tr_pos(2) = state.position_z - node.position_z;
	
	vec loc_pos = node.axis * tr_pos;
	
	local_position_x = loc_pos(0);
	local_position_y = loc_pos(1);
	local_position_z = loc_pos(2);
	
	// Momentum
	
	vec mom = zeros<vec>(3);
	mom(0) = state.momentum_x;
	mom(1) = state.momentum_y;
	mom(2) = state.momentum_z;
	
	vec loc_mom = node.axis * mom;
	
	local_momentum_x = loc_mom(0);
	local_momentum_y = loc_mom(1);
	local_momentum_z = loc_mom(2);
}

void state_local_to_global(
	ParticleState& state,
	Node& node, 
	double local_position_x, 
	double local_position_y, 
	double local_position_z,
	double local_momentum_x,
	double local_momentum_y,
	double local_momentum_z)
{
	vec loc_pos = zeros<vec>(3);
	loc_pos(0) = local_position_x;
	loc_pos(1) = local_position_y;
	loc_pos(2) = local_position_z;
	
	vec pos = node.axis.t() * loc_pos;
	
	state.position_x = pos(0) + node.position_x;
	state.position_y = pos(1) + node.position_y;
	state.position_z = pos(2) + node.position_z;
	
	// Momentum
	vec loc_mom = zeros<vec>(3);
	loc_mom(0) = local_momentum_x;
	loc_mom(1) = local_momentum_y;
	loc_mom(2) = local_momentum_z;
	
	vec mom = node.axis.t() * loc_mom;
	
	state.momentum_x = mom(0);
	state.momentum_y = mom(1);
	state.momentum_z = mom(2);
}

void check_lua_error(char** lua_msg)
{
    if (*lua_msg != NULL)
    {
        printf("Error while parsing/executing LUA script at:\n%s\n", *lua_msg);
        exit(-1);
    }
}

/**
 * 
 * Trasform a number into a string using SI prefixes:
 * 
 * 0.005 -> 5.000 m
 * 5000  -> 5.000 k
 */
//string format_with_suffix()
//{
//	
//	}



/**
 * Scale an image and ensure it will never be bigger than max_w x max_h.
 * It will scale it keeping the proportions
 */
void scale_image(unsigned int& w, unsigned int& h, unsigned int max_w, unsigned int max_h)
{

	double ratio = 1.0 * max_w / max_h;
	
	unsigned int orig_w = w;
	unsigned int orig_h = h;

	if (1.d * orig_w / orig_h >= ratio)
	{
		if (orig_w > max_w)
		{
			w = max_w;
			h = (orig_h * w / orig_w / 2) * 2; // Ensure we have an even number
		}
		else
		{
			w = (orig_w / 2) * 2;
			h = (orig_h / 2) * 2;
		}
	}
	else
	{
		if (orig_h > max_h)
		{
			h = max_h;
			w = (orig_w * h / orig_h / 2) * 2; // Ensure we have an even number
		}
		else
		{
			h = (orig_h / 2) * 2;
			w = (orig_w / 2) * 2;
		}
	}
}
