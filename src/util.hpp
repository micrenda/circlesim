#include "type.hpp"

template <typename T> T vector_module(T x1, T x2, T x3);

double energy_kinetic_to_momentum(double rest_mass, double energy_kinetic);
double momentum_to_energy_kinetic(double rest_mass, double momentum);
double energy_total_to_momentum(double rest_mass, double energy);
double momentum_to_energy_total(double rest_mass, double momentum);

template <typename T> void spherical_to_cartesian(T rho, T theta, T phi, T& x, T& y, T& z);
template <typename T> void cartesian_to_spherical(T x, T y, T z, T& theta, T& phi);

template <typename T> void rotate_spherical	(T& x, T& y, T& z, T theta, T phi);
template <typename T> void rotate_euler		(T& x, T& y, T& z, T alpha, T beta, T gamma);

void state_global_to_local(ParticleStateLocal&  state_local,  ParticleStateGlobal& state_global, Node& node);
void state_local_to_global(ParticleStateGlobal& state_global, ParticleStateLocal&  state_local,  Node& node);

void check_lua_error(char** lua_msg);

void scale_image(unsigned int& w, unsigned int& h, unsigned int max_w, unsigned int max_h);




template <typename T> T vector_module(T x1, T x2, T x3)
{
	return hypot(hypot(x1, x2), x3);
}


template <typename T> void unique_spherical_angles(T& theta, T& phi)
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
template <typename T> void unique_spherical_all(T& module, T& theta, T& phi)
{
	if (module < 0)
	{
		module = -module;
		theta += 2 * M_PI;
	}
	
	 unique_spherical_angles(theta, phi);
}



template<typename T> void spherical_to_cartesian(T rho, T theta, T phi, T& x, T& y, T& z)
{
	x = rho * sin(theta) * cos(phi);
	y = rho * sin(theta) * sin(phi);
	z = rho * cos(theta);
}

template<typename T> void cartesian_to_spherical(T x, T y, T z, T& theta, T& phi)
{
	theta	= acos(z / sqrt(x*x + y*y + z*z));
	
	if (y == 0 && x == 0)
		phi		= 0; // We cover the case when we select the nord or south pole
	else
		phi		= atan(y / x);
	
	unique_spherical_angles(theta, phi);
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
template <typename T> void rotate_euler(T& x, T& y, T& z, T alpha, T beta, T gamma)
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



// Spherical rotation. TODO: write documentation
template <typename T> void rotate_spherical(T& x, T& y, T& z, T theta, T phi)
{
	T buffer_rho;
	T buffer_theta;
	T buffer_phi;
	
	cartesian_to_spherical(x, y, z, buffer_theta, buffer_phi);
	
	buffer_theta 	+= theta;
	buffer_phi		+= phi;
	
	unique_spherical_angles(buffer_theta, buffer_phi);
	
	buffer_rho = vector_module(x, y, z);
	
	spherical_to_cartesian(buffer_rho, buffer_theta, buffer_phi, x, y, z);
}


string get_thread_prefix(int h);

 
