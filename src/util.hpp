#include "type.hpp"

int 		vector_module(int x1, 			int x2,			int x3);
long 		vector_module(long x1, 			long x2,		long x3);
double 		vector_module(double x1, 		double x2,		double x3);
long double vector_module(long double x1,	long double x2,	long double x3);



double energy_to_momentum(double rest_mass, double energy);
double momentum_to_energy(double rest_mass, double momentum);

void spherical_to_cartesian(double rho, double theta, double phi, double& x, double& y, double& z);
void cartesian_to_spherical(double x, double y, double z, double& theta, double& phi);

void rotate_spherical	(double& x, double& y, double& z, double theta, double phi);
void rotate_euler		(double& x, double& y, double& z, double alpha, double beta, double gamma);

void state_global_to_local(ParticleStateLocal&  state_local,  ParticleStateGlobal& state_global, Node& node);
void state_local_to_global(ParticleStateGlobal& state_global, ParticleStateLocal&  state_local,  Node& node);

void check_lua_error(char** lua_msg);

void scale_image(unsigned int& w, unsigned int& h, unsigned int max_w, unsigned int max_h);
