#include <boost/algorithm/string/regex.hpp>


#include "gradient.hpp"
#include "type.hpp"


using namespace bo;

Gradient::Gradient(std::string& s, double value_min, double value_max, double value_min_abs, double value_max_abs, bool debug/* = false*/)
{
	
	static const bo::regex regex_color("^\\#([0-9a-f]{1,6})(\\(([\\+\\-])?([\\_a-z]+)\\))?$");
	
	erase_all(s, " ");
	to_lower(s);

	vector<string> tokens;
	bo::algorithm::split_regex( tokens, s, regex("\\-\\-") );
	
	for (string token: tokens)
	{
		match_results<std::string::const_iterator> what;
		
		GradientItem item;
		
		if (token.empty())
		{
			items.push_back(item);
		}
		else if (regex_match(token, what, regex_color))
		{
			item.has_color = true;
			item.color = stoul(what[1], 0, 16);
			
			if (!string(what[2]).empty())
			{
				
				string value_str = what[4];
				double value;
				
				if (value_str == "zero")
					value = 0.d;
				else if (value_str == "min")
					value = value_min;
				else if (value_str == "max")
					value = value_max;
				else if (value_str == "min_abs")
					value = value_min_abs;
				else if (value_str == "max_abs")
					value = value_max_abs;
				else
				{
					printf("Unable understand gradient value '%s'\n", value_str.c_str());
					exit(-1);
				}
					
					
				bool positive  = true;
				
				if (what[3] == "-") positive = false;
				
				if (!positive)
					value *= -1.d;
					
				item.has_value = true;
				item.value     = value;
				
			}
			
			items.push_back(item);
		}
		
	}
	
	if (items.size() < 2)
	{
		printf("A gradient must have at least 2 values.\n");
		exit(-1);
	}
	
	if (!items[0].has_color)
	{
		printf("A gradient must have the first color specified\n");
		exit(-1);
	}
	
	if (!items[items.size()-1].has_color)
	{
		printf("A gradient must have the last color specified\n");
		exit(-1);
	}
	
	if (!items[0].has_value)
	{
		items[0].has_value = true;
		items[0].value = value_min;
	}
	
	if (!items[items.size()-1].has_value)
	{
		items[items.size()-1].has_value = true;
		items[items.size()-1].value = value_max;
	}
	
	unsigned int last_i;
	
	// Filling the missing values
	last_i = 0;
	
	for (unsigned int i = last_i + 1; i < items.size(); i++)
	{
		if (items[i].has_value)
		{
			
			for (unsigned int j = last_i + 1; j < i; j++)
			{
				if (!items[j].has_value)
				{
					items[j].value = items[last_i].value + (items[i].value - items[last_i].value) * (j - last_i) / (i - last_i);
					items[j].has_value = true;
				}
			}
			
			last_i = i;
		}
		
	}
	
	// Filling the missing colors
	last_i = 0;
	
	for (unsigned int i = last_i + 1; i < items.size(); i++)
	{
		if (items[i].has_color)
		{
			for (unsigned int j = last_i + 1; j < i; j++)
			{
				if (!items[j].has_color)
				{
					unsigned char from_r 	= (items[last_i].color & 0xff0000) >> 16;
					unsigned char from_g 	= (items[last_i].color & 0x00ff00) >>  8;
					unsigned char from_b 	= (items[last_i].color & 0x0000ff) >>  0;
					unsigned char to_r	 	= (items[     i].color & 0xff0000) >> 16;
					unsigned char to_g	 	= (items[     i].color & 0x00ff00) >>  8;
					unsigned char to_b	 	= (items[     i].color & 0x0000ff) >>  0;
					
					unsigned char current_r = from_r + (to_r - from_r) * (j - last_i) / (i - last_i);
					unsigned char current_g = from_g + (to_g - from_g) * (j - last_i) / (i - last_i);
					unsigned char current_b = from_b + (to_b - from_b) * (j - last_i) / (i - last_i);
					
					items[j].color = (current_r << 16) |  (current_g <<  8) | (current_b <<  0);
					items[j].has_color = true;
				}
			}
			last_i = i;
		}
		
	}
	
	
	// Checking if the gradient is healty
	unsigned int errors = 0;
	for (unsigned int i = 0; i < items.size(); i++)
	{
		if (!items[i].has_value)
		{
			printf("Internal error: In gradient definition the element %u has no value.\n", i);
			errors++;	
		}
		if (!items[i].has_color)
		{
			printf("Internal error: In gradient definition the element %u has no color.\n", i);
			errors++;	
		}
		
		if (i > 0 && items[i-1].value > items[i].value)
		{
			printf("Internal error: In gradient definition the element %u has a value bigger than element %u.\n", i, i + 1);
			errors++;
		}
	}
	
	if (errors > 0)
		exit(-1);
	
	if (debug)
	{
		printf("Gradient composed by %u elements:\n", (unsigned int)items.size());
		for (GradientItem& item: items)
		{	
			if (item.has_value && item.has_color)
				printf("Element %E: #%06x\n", item.value, item.color);
			else if (item.has_value && !item.has_color)
				printf("Element %E: (no-color)\n", item.value);
			else if (!item.has_value && item.has_color)
				printf("Element (no-value): #%06x\n", item.color);
			else if (!item.has_value && !item.has_color)
				printf("Element (no-value): (no-color)\n");
			
		}
	}
}



unsigned int Gradient::interpolate_color(double value_from, double value_to, double value_current, unsigned int color_from, unsigned int color_to)
{
	unsigned char from_r 	= (color_from & 0xff0000) >> 16;
	unsigned char from_g 	= (color_from & 0x00ff00) >>  8;
	unsigned char from_b 	= (color_from & 0x0000ff) >>  0;
	unsigned char to_r	 	= (color_to   & 0xff0000) >> 16;
	unsigned char to_g	 	= (color_to   & 0x00ff00) >>  8;
	unsigned char to_b	 	= (color_to   & 0x0000ff) >>  0;
	
	double delta = (value_current - value_from) / (value_to - value_from);
	
	unsigned char current_r = from_r + (to_r - from_r) * delta;
	unsigned char current_g = from_g + (to_g - from_g) * delta;
	unsigned char current_b = from_b + (to_b - from_b) * delta;
				
	return (current_r << 16) |  (current_g <<  8) | (current_b <<  0);
	
}

unsigned int Gradient::get_color(double value)
{
	unsigned int s = items.size();
	
	for (unsigned int i = 0; i < s; i++)
	{
		if (i == 0 && items[i].value > value)
			return items[i].color;
		if (i == s - 1 && items[i].value <= value)
			return items[s].color;
		else if (items[i].value <= value && items[i+1].value > value)
			return interpolate_color(items[i].value, items[i+1].value, value, items[i].color, items[i+1].color);
		
		
	}
	return 0;
}

double Gradient::get_lowest_value()
{
	return items[0].value;
}

double Gradient::get_highest_value()
{
	return items[items.size()-1].value;
}
