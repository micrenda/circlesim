#include "type.hpp"

int 		vector_module(int x1, 			int x2,			int x3);
long 		vector_module(long x1, 			long x2,		long x3);
double 		vector_module(double x1, 		double x2,		double x3);
long double vector_module(long double x1,	long double x2,	long double x3);



double energy_to_momentum(double rest_mass, double energy);

void spherical_to_cartesian(double rho, double theta, double phi, double& x, double& y, double& z);
void cartesian_to_spherical(double x, double y, double z, double& theta, double& phi);

void rotate_spherical	(double& x, double& y, double& z, double theta, double phi);
void rotate_euler		(double& x, double& y, double& z, double alpha, double beta, double gamma);

void state_global_to_local(ParticleState& state, Node& node, double local_position_x, double local_position_y, double local_position_z);
void state_local_to_global(ParticleState& state, Node& node, double local_position_x, double local_position_y, double local_position_z);

void state_global_to_local(	ParticleState& state, 	Node& node,	double& local_position_x,	double& local_position_y,	double& local_position_z,	double& local_momentum_x,	double& local_momentum_y,	double& local_momentum_z);
void state_local_to_global(	ParticleState& state,	Node& node, double  local_position_x, 	double  local_position_y, 	double  local_position_z,	double  local_momentum_x,	double  local_momentum_y,	double  local_momentum_z);

void check_lua_error(char** lua_msg);

void scale_image(unsigned int& w, unsigned int& h, unsigned int max_w, unsigned int max_h);
