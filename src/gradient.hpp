#ifndef GRADIENT_H
#define GRADIENT_H

#include <vector>
#include <string>
using namespace std;


class GradientItem
{
	public:
		bool   has_value = false;
		bool   has_color = false;
		
		double value   		= 0.d;
		unsigned int color 	= 0x000000;
};

class Gradient
{
	private:
		Gradient() { }
		
		vector<GradientItem> items;
		unsigned int interpolate_color(double value_from, double value_to, double value_current, unsigned int color_from, unsigned int color_to);
		
	public:
		Gradient (string s, double value_min, double value_max, double value_min_abs, double value_max_abs);
		unsigned int get_color     (double value);
		double       get_percentual(double value);
};

#endif
