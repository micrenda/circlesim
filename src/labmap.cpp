#include <png++/png.hpp>
#include "util.hpp"
#include "type.hpp"
using namespace png;

typedef struct LabSize
{
	double min_1;
	double min_2;
	double max_1;
	double max_2;

	double size_1;
	double size_2 ;
	
	double dr;
} LabSize;

unsigned int get_i(LabSize& lab_size, double position)
{
	return trunc((position - lab_size.min_1)/lab_size.dr);
}

unsigned int get_j(LabSize& lab_size, double position)
{
	return trunc((position - lab_size.min_2)/lab_size.dr);
}

double get_position_1(LabSize& lab_size, double pixel)
{
	return lab_size.min_1 + pixel * lab_size.dr;
}

double get_position_2(LabSize& lab_size, double pixel)
{
	return lab_size.min_2 + pixel * lab_size.dr;
}

void get_node_center(Node& node, short axis_1, short axis_2, double& center_1, double& center_2)
{
	switch (axis_1)
	{
		case 1:
			center_1 = node.position_x;
		break;
		case 2:
			center_1 = node.position_y;
		break;
		case 3:
			center_1 = node.position_z;
		break;
	}
	
	switch (axis_2)
	{
		case 1:
			center_2 = node.position_x;
		break;
		case 2:
			center_2 = node.position_y;
		break;
		case 3:
			center_2 = node.position_z;
		break;
	}
}

void get_particle_position(ParticleStateGlobal& state, short axis_1, short axis_2, double& position_1, double& position_2)
{
	switch (axis_1)
	{
		case 1:
			position_1 = state.position_x;
		break;
		case 2:
			position_1 = state.position_y;
		break;
		case 3:
			position_1 = state.position_z;
		break;
	}

	switch (axis_2)
	{
		case 1:
			position_2 = state.position_x;
		break;
		case 2:
			position_2 = state.position_y;
		break;
		case 3:
			position_2 = state.position_z;
		break;
	}
}

void compute_lab_limits(Laboratory& laboratory, Simulation& simulation, vector<SimluationResultFreeSummary>& summaries_free, short axis_1, short axis_2, LabSize& lab_size)
{
	lab_size.min_1 = +INFINITY;
	lab_size.min_2 = +INFINITY;
	lab_size.max_1 = -INFINITY;
	lab_size.max_2 = -INFINITY;

	for (Node& node: laboratory.nodes)
	{
		double center_1;
		double center_2;
		 
		get_node_center(node, axis_1, axis_2, center_1, center_2);
		
		if (lab_size.min_1 > center_1)
			lab_size.min_1 = center_1;
		if (lab_size.min_2 > center_2)
			lab_size.min_2 = center_2;
		if (lab_size.max_1 < center_1)
			lab_size.max_1 = center_1;
		if (lab_size.max_2 < center_2)
			lab_size.max_2 = center_2;	
	}
	
	for (SimluationResultFreeSummary& summary_free: summaries_free)
	{
		for (SimluationResultFreeItem& item: summary_free.items)
		{
			double position_1, position_2;
			get_particle_position(item.state, axis_1, axis_2, position_1, position_2);
		
			if (lab_size.min_1 > position_1)
				lab_size.min_1 = position_1;
			if (lab_size.min_2 > position_2)
				lab_size.min_2 = position_2;
			if (lab_size.max_1 < position_1)
				lab_size.max_1 = position_1;
			if (lab_size.max_2 < position_2)
				lab_size.max_2 = position_2;	
		}
	}
	
	lab_size.min_1 -= 1.5 * simulation.laser_influence_radius;
	lab_size.min_2 -= 1.5 * simulation.laser_influence_radius;
	lab_size.max_1 += 1.5 * simulation.laser_influence_radius;
	lab_size.max_2 += 1.5 * simulation.laser_influence_radius;
	
	lab_size.size_1 = lab_size.max_1 - lab_size.min_1;
	lab_size.size_2 = lab_size.max_2 - lab_size.min_2;
	
	if (lab_size.size_1 >= lab_size.size_2)
		lab_size.dr = lab_size.size_1 / simulation.max_labmap_size;
	else
		lab_size.dr = lab_size.size_2 / simulation.max_labmap_size;
	
}

void draw_base(image<rgb_pixel>& image_base, unsigned int count_i, unsigned int count_j, Simulation& simulation, Laboratory& laboratory, LabSize& lab_size, short axis_1, short axis_2)
{
	const rgb_pixel fg = rgb_pixel(127, 127, 127);
	const rgb_pixel bg  = rgb_pixel(255, 255, 255);
	
	for (Node& node: laboratory.nodes)
	{
		double center_1, center_2;
		get_node_center(node, axis_1, axis_2, center_1, center_2);
		
		unsigned int center_i = get_i(lab_size, center_1);
		unsigned int center_j = get_j(lab_size, center_2);
		
		int radius = trunc(simulation.laser_influence_radius / lab_size.dr);
		
		for (unsigned int i = 0; i < count_i; i++)
		{
			for (unsigned int j = 0; j < count_j; j++)
			{
				int delta_i = center_i - i;
				int delta_j = center_j - j;
				
				if (vector_module(delta_i, delta_j, 0) == radius)
					image_base[i][j] = fg;
				else if (abs(delta_i) <= 3  && center_j == j)
					image_base[i][j] = fg;
				else if (center_j == i && abs(delta_j) <= 3)
					image_base[i][j] = fg;
				else if (i == 0 || i == count_i - 1)
					image_base[i][j] = fg;
				else if (j == 0 || j == count_j - 1)
					image_base[i][j] = fg;
				else
					image_base[i][j] = bg;
			}	
		}
	}
}

void draw_particle(image<rgb_pixel>& image, unsigned int count_i, unsigned int count_j, ParticleStateGlobal& state, LabSize& lab_size, short axis_1, short axis_2)
{
	const rgb_pixel pixel_particle = rgb_pixel(255, 0, 100);
	
	double position_1, position_2;
	get_particle_position(state, axis_1, axis_2, position_1, position_2);
	
	unsigned int i = get_i(lab_size, position_1);
	unsigned int j = get_j(lab_size, position_2);
	
	image[i][j] = pixel_particle;
	
	if (i >= 1)
		image[i-1][j] = pixel_particle;
	if (i <= count_i - 1)
		image[i+1][j] = pixel_particle;
	if (j >= 1)
		image[i][j-1] = pixel_particle;
	if (j <= count_j - 1)
		image[i][j+1] = pixel_particle;
	
}

string get_axis(short id)
{
	switch (id)
	{
		case 1:
			return "x";
		case 2:
			return "y";
		case 3:
			return "z";
		default:
			return "?";
	}
}

void render_labmap(Laboratory& laboratory, Simulation& simulation, Pulse& laser, vector<SimluationResultFreeSummary>& summaries_free, vector<SimluationResultNodeSummary>& summaries_node, short axis_1, short axis_2, fs::path output_dir)
{
	
	LabSize lab_size;
	compute_lab_limits(laboratory, simulation, summaries_free, axis_1, axis_2, lab_size);
	
	unsigned int count_i = trunc(lab_size.size_1 / lab_size.dr);
	unsigned int count_j = trunc(lab_size.size_2 / lab_size.dr);
	
	
	image<rgb_pixel> frame_base(count_i, count_j);
	draw_base(frame_base, count_i, count_j, simulation, laboratory, lab_size, axis_1, axis_2);
	frame_base.write((output_dir / fs::path((bo::format("labmap_%s%s_base.png") % get_axis(axis_1) % get_axis(axis_2)).str())).string());

	unsigned long t = 0;
	
	for (SimluationResultFreeSummary& summary_free: summaries_free)
	{
		for (SimluationResultFreeItem& item: summary_free.items)
		{
			image<rgb_pixel> frame(count_i, count_j);
			
			for (unsigned int i = 0; i < count_i; i++)
				for (unsigned int j = 0; j < count_j; j++)
					frame[i][j] = frame_base[i][j];
					
			draw_particle(frame, count_i, count_j, item.state, lab_size, axis_1, axis_2);
			
			frame.write((output_dir / fs::path((bo::format("labmap_%s%s_t%u.png") % get_axis(axis_1) % get_axis(axis_2) % t).str())).string());
			
			t++;
		}
	}
}

