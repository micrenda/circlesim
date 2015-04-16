#include <math.h>
#include <armadillo>
#include "type.hpp"
#include "util.hpp"


using namespace arma;

double energy_kinetic_to_momentum(double rest_mass, double energy_kinetic)
{
	// Energy E = E₀ + Eₖ

	double energy_total = rest_mass * C0 * C0 + energy_kinetic;
	return energy_total_to_momentum(rest_mass, energy_total);
}

double momentum_to_energy_kinetic(double rest_mass, double momentum)
{
	double energy_total = momentum_to_energy_total(rest_mass, momentum);
	return energy_total - rest_mass * C0 * C0;
}

double energy_total_to_momentum(double rest_mass, double energy)
{
	//  We use these formule to find the relativistic momentum:
	//  p = mv      E=mc²       where m = m₀ + mₖ
	//  We define these two units:
	//  β = v/c             γ = 1 / √(1-v²/c²)
	//  Combining the last two definitions we get
	//  β = √(1-1/γ²)       γ = 1 / √(1-β²) 
	
	
	// Let's start:
	//   p = mv = m₀γv = m₀γcβ
	// Extracting γ from E we get:
	//   γ = Ε/(m₀c²)
	double gamma = energy / (rest_mass * C0 * C0);
	// So now we know all except β. We can extract it from gamma using previous relations
	//   β = √(1-1/γ²)
	double beta = sqrt(1.d-1.d/(gamma*gamma));
	// We can now get p:
	return rest_mass * gamma * C0 * beta;
}

double momentum_to_energy_total(double rest_mass, double momentum)
{
	//  We use these formule to find the relativistic momentum:
	//  p = mv      E=mc²       where m = m₀ + mₖ
	//  We define these two units:
	//  β = v/c             γ = 1 / √(1-v²/c²)
	//  Combining the last two definitions we get
	//  β = √(1-1/γ²)       γ = 1 / √(1-β²) 
	
	// Let's start:
	//   E = mc² = m₀γc²
	// Extracting γ from p we get:
	// γ =  √{1+[p/(m₀c)]²}, or defining ζ = p/(m₀c) we have γ =  √{1+ζ²}
	//
	double zeta = momentum / (rest_mass * C0);
	double gamma = sqrt(1 + zeta * zeta);
	// We now can get E
	return rest_mass * gamma * C0 * C0;
}

void state_global_to_local(ParticleStateLocal&  state_local, ParticleStateGlobal& state_global, Node& node)
{
	// Position
	
	vec tr_pos = zeros<vec>(3);
	tr_pos(0) = state_global.position_x - node.position_x;
	tr_pos(1) = state_global.position_y - node.position_y;
	tr_pos(2) = state_global.position_z - node.position_z;
	
	vec loc_pos = node.axis * tr_pos;
	
	state_local.position_x = loc_pos(0);
	state_local.position_y = loc_pos(1);
	state_local.position_z = loc_pos(2);
	
	// Momentum
	
	vec mom = zeros<vec>(3);
	mom(0) = state_global.momentum_x;
	mom(1) = state_global.momentum_y;
	mom(2) = state_global.momentum_z;
	
	vec loc_mom = node.axis * mom;
	
	state_local.momentum_x = loc_mom(0);
	state_local.momentum_y = loc_mom(1);
	state_local.momentum_z = loc_mom(2);
}

void state_local_to_global(ParticleStateGlobal& state_global, ParticleStateLocal& state_local, Node& node)
{
	vec loc_pos = zeros<vec>(3);
	loc_pos(0) = state_local.position_x;
	loc_pos(1) = state_local.position_y;
	loc_pos(2) = state_local.position_z;
	
	vec pos = node.axis.t() * loc_pos;
	
	state_global.position_x = pos(0) + node.position_x;
	state_global.position_y = pos(1) + node.position_y;
	state_global.position_z = pos(2) + node.position_z;
	
	// Momentum
	vec loc_mom = zeros<vec>(3);
	loc_mom(0) = state_local.momentum_x;
	loc_mom(1) = state_local.momentum_y;
	loc_mom(2) = state_local.momentum_z;
	
	vec mom = node.axis.t() * loc_mom;
	
	state_global.momentum_x = mom(0);
	state_global.momentum_y = mom(1);
	state_global.momentum_z = mom(2);
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
//  }



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
