#include "type.hpp"
double get_attribute(Particle& particle, ParticleStateGlobal& particle_state, Pulse& laser, string object, string attribute);
void   set_attribute(Particle& particle, ParticleStateGlobal& particle_state, Pulse& laser, string object, string attribute, double new_value);
string get_conversion_si_unit(string object, string attribute);
double get_conversion_si_value(string object, string attribute);
