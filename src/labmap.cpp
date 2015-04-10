#include <png++/png.hpp>
#include "util.hpp"
#include "type.hpp"
#include "simulator.hpp"
#include "plot.hpp"

using namespace png;

typedef struct LabSizeGlobal
{
	double min_x;
	double max_x;
	double size_x;
	
	double min_y;
	double max_y;
	double size_y;
	
	double min_z;
	double max_z;
	double size_z;
	
	double dr;
} LabSizeGlobal;

typedef struct LabSizePlane
{
	double min_1;
	double min_2;
	
	double max_1;
	double max_2;

	double size_1;
	double size_2;
	
	double dr;
} LabSizePlane;

const rgb_pixel color_fg 		= rgb_pixel(63, 63, 63);
const rgb_pixel color_bg  		= rgb_pixel(255, 255, 255);
const rgb_pixel color_particle 	= rgb_pixel(255, 0, 100);

int get_i(LabSizePlane& lab_size, double position)
{
	return trunc((position - lab_size.min_1)/lab_size.dr);
}

int get_j(LabSizePlane& lab_size, double position)
{
	return trunc((position - lab_size.min_2)/lab_size.dr);
}

double get_position_1(LabSizePlane& lab_size, double pixel)
{
	return lab_size.min_1 + pixel * lab_size.dr;
}

double get_position_2(LabSizePlane& lab_size, double pixel)
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

void compute_lab_limits(Laboratory& laboratory, Simulation& simulation, vector<SimluationResultFreeSummary>& summaries_free, LabSizeGlobal& lab_size)
{
	lab_size.min_x = +INFINITY;
	lab_size.min_y = +INFINITY;
	lab_size.min_z = +INFINITY;
	
	lab_size.max_x = -INFINITY;
	lab_size.max_y = -INFINITY;
	lab_size.max_z = -INFINITY;

	for (Node& node: laboratory.nodes)
	{
		if (lab_size.min_x > node.position_x)
			lab_size.min_x = node.position_x;
		if (lab_size.min_y > node.position_y)
			lab_size.min_y = node.position_y;
		if (lab_size.min_z > node.position_z)
			lab_size.min_z = node.position_z;
			
		if (lab_size.max_x < node.position_x)
			lab_size.max_x = node.position_x;
		if (lab_size.max_y < node.position_y)
			lab_size.max_y = node.position_y;
		if (lab_size.max_z < node.position_z)
			lab_size.max_z = node.position_z;	
	}
	
	if (simulation.labmap_full)
	{
		for (SimluationResultFreeSummary& summary_free: summaries_free)
		{
			for (SimluationResultFreeItem& item: summary_free.items)
			{
				ParticleStateGlobal& state = item.state;
				
				if (lab_size.max_x < state.position_x)
					lab_size.max_x = state.position_x;
				if (lab_size.max_y < state.position_y)
					lab_size.max_y = state.position_y;
				if (lab_size.max_z < state.position_z)
					lab_size.max_z = state.position_z;		
			}
		}
	}
	
	lab_size.min_x -= 2.0 * simulation.laser_influence_radius;
	lab_size.min_y -= 2.0 * simulation.laser_influence_radius;
	lab_size.min_z -= 2.0 * simulation.laser_influence_radius;
	
	lab_size.max_x += 2.0 * simulation.laser_influence_radius;
	lab_size.max_y += 2.0 * simulation.laser_influence_radius;
	lab_size.max_z += 2.0 * simulation.laser_influence_radius;
	
	lab_size.size_x = lab_size.max_x - lab_size.min_x;
	lab_size.size_y = lab_size.max_y - lab_size.min_y;
	lab_size.size_z = lab_size.max_z - lab_size.min_z;
	
	lab_size.dr = max(max(lab_size.size_x, lab_size.size_y), lab_size.size_z) / simulation.labmap_max_size;
}

bool inside_lab_size(ParticleStateGlobal& state, LabSizeGlobal& lab_size)
{
	bool inside = true;
	
	inside &= state.position_x >= lab_size.min_x && state.position_x <= lab_size.max_x;
	inside &= state.position_y >= lab_size.min_y && state.position_y <= lab_size.max_y;
	inside &= state.position_z >= lab_size.min_z && state.position_z <= lab_size.max_z;
	
	return inside;
}

LabSizePlane get_lab_size(LabSizeGlobal& global, short axis_1, short axis_2)
{
	LabSizePlane plane;
	
	switch(axis_1)
	{
		case 1:
			plane.min_1	 = global.min_x;
			plane.max_1	 = global.max_x;
			plane.size_1 = global.size_x;
		break;
		
		case 2:
			plane.min_1	 = global.min_y;
			plane.max_1	 = global.max_y;
			plane.size_1 = global.size_y;
		break;
		
		case 3:
			plane.min_1	 = global.min_z;
			plane.max_1	 = global.max_z;
			plane.size_1 = global.size_z;
		break;
	}
	
	switch(axis_2)
	{
		case 1:
			plane.min_2	 = global.min_x;
			plane.max_2	 = global.max_x;
			plane.size_2 = global.size_x;
		break;
		
		case 2:
			plane.min_2	 = global.min_y;
			plane.max_2	 = global.max_y;
			plane.size_2 = global.size_y;
		break;
		
		case 3:
			plane.min_2	 = global.min_z;
			plane.max_2	 = global.max_z;
			plane.size_2 = global.size_z;
		break;
	}
	
	plane.dr = global.dr;
	
	return plane;
}


void draw_base(image<rgb_pixel>& image_base, int count_i, int count_j, Simulation& simulation, Laboratory& laboratory, LabSizePlane& lab_size, short axis_1, short axis_2)
{
	for (int i = 0; i < count_i; i++)
		for (int j = 0; j < count_j; j++)
			image_base[j][i] = color_bg;
			
				
	for (Node& node: laboratory.nodes)
	{
		double center_1, center_2;
		get_node_center(node, axis_1, axis_2, center_1, center_2);

		int center_i = get_i(lab_size, center_1);
		int center_j = get_j(lab_size, center_2);
		
		int radius = trunc(simulation.laser_influence_radius / lab_size.dr);
		
		for (int i = 0; i < count_i; i++)
		{
			int delta_i = center_i - i;
			for (int j = 0; j < count_j; j++)
			{
				int delta_j = center_j - j;
				
				// Draw circle border
				int module = vector_module(delta_i, delta_j, 0);
				if (module == radius)
					image_base[j][i] = color_fg;
				
				// Draw cross in the center
				if ((abs(delta_i) <= radius / 10 && center_j == j) || (abs(delta_j) <= radius / 10 && center_i == i))
					image_base[j][i] = color_fg;
				
					
					
			}	
		}
	}
}

void draw_particle(image<rgb_pixel>& image, int count_i, int count_j, ParticleStateGlobal& state, LabSizePlane& lab_size, short axis_1, short axis_2)
{
	double position_1, position_2;
	get_particle_position(state, axis_1, axis_2, position_1, position_2);
	
	int particle_radius = ceil(min(count_i, count_j) / 250.d);
	
	int particle_i = get_i(lab_size, position_1);
	int particle_j = get_j(lab_size, position_2);
	
	for (int i = 0; i < count_i; i++)
	{
		int delta_i = i - particle_i;
		for (int j = 0; j < count_j; j++)
		{
			int delta_j = j - particle_j;
			if (delta_i*delta_i + delta_j*delta_j < particle_radius * particle_radius)
			{
				image[j][i] = color_particle;	
			}
		}
	}
}


void get_node_field(Field& field, Node& node, Pulse& laser, LabSizePlane& lab_size, int i, int j, short axis_1, short axis_2, double local_time, FunctionFieldType function_field)
{
	double local_position_x;
	double local_position_y;
	double local_position_z;
	
	if (axis_1 == 1 && axis_2 == 2)
	{
		local_position_x =  i * lab_size.dr;
		local_position_y =  j * lab_size.dr;
		local_position_z =  0;
	}
	else if (axis_1 == 2 && axis_2 == 1)
	{
		local_position_x =  j * lab_size.dr;
		local_position_y =  i * lab_size.dr;
		local_position_z =  0;
	}
	else if (axis_1 == 1 && axis_2 == 3)
	{
		local_position_x =  i * lab_size.dr;
		local_position_y =  0;
		local_position_z =  j * lab_size.dr;
	}
	else if (axis_1 == 3 && axis_2 == 1)
	{
		local_position_x =  j * lab_size.dr;
		local_position_y =  0;
		local_position_z =  i * lab_size.dr;
	}
	else if (axis_1 == 2 && axis_2 == 3)
	{
		local_position_x =  0;
		local_position_y =  i * lab_size.dr;
		local_position_z =  j * lab_size.dr;
	}
	else if (axis_1 == 3 && axis_2 == 2)
	{
		local_position_x =  0;
		local_position_y =  j * lab_size.dr;
		local_position_z =  i * lab_size.dr;
	}
	else
	{
		printf("ERROR - Unexprect combination of axis1 and axis2. Contact developer.\n");
		exit(-2);
	}

	calculate_fields(local_time * C0, local_position_x, local_position_y, local_position_z, laser, field, function_field);
}


void draw_field(image<rgb_pixel>& image, int count_i, int count_j, SimluationResultNodeItem& item, Node& node, LabMapLimit& limit, LabSizePlane& lab_size, short axis_1, short axis_2, Simulation& simulation, Pulse& laser, FunctionFieldType function_field)
{
	double center_1, center_2;
	get_node_center(node, axis_1, axis_2, center_1, center_2);
	
	int global_center_i = get_i(lab_size, center_1);
	int global_center_j = get_j(lab_size, center_2);
	
	int radius = trunc(simulation.laser_influence_radius / lab_size.dr);
	
	for (int i = -radius + 1; i < radius - 1; i++)
	{
		for (int j = -radius + 1; j < radius - 1; j++)
		{
			if (i * i + j * j  < radius * radius)
			{
				rgb_pixel current_color = image[global_center_j+j][global_center_i+i];
				if (current_color.red == color_bg.red && current_color.green == color_bg.green && current_color.blue == color_bg.blue)
				{
					Field field;
					get_node_field(field, node, laser, lab_size, i, j, axis_1, axis_2, item.local_time, function_field);
					
					double value     = vector_module(field.e_x, field.e_y, field.e_z);
					double value_max = limit.e_mod_max;
					
					unsigned short red   = 0xff;
					unsigned short green = 0xff;
					unsigned short blue  = 0xff;
					
					if (value_max > 0)
					{
						red   = 255 - round(0xff * value/value_max);
						green = 255 - round(0x7f * value/value_max);
						blue  = 255 - round(0x00 * value/value_max);
					}
					
					image[global_center_j+j][global_center_i+i] = rgb_pixel(red, green, blue);
				}
			}
		}
	}
	
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

void flip_imabe_v(image<rgb_pixel>& image, int count_i, int count_j)
{
	for (int i=0; i < count_i; i++)
	{
		for (int j=0; j < count_j/2; j++)
		{
			rgb_pixel buffer;
			
			buffer = image[j][i];
			image[j][i] = image[count_j-j- 1][i];
			image[count_j-j-1][i] = buffer;
		}
	}
}

void draw_free(image<rgb_pixel> frame_base, SimluationResultFreeSummary& summary_free, LabSizePlane& lab_size, int count_i, int count_j, short axis_1, short axis_2, long unsigned int& t, Simulation& simulation, LabSizeGlobal& lab_size_global, fs::path output_dir)
{
	for (SimluationResultFreeItem& item: summary_free.items)
	{
		if (simulation.labmap_full || inside_lab_size(item.state, lab_size_global))
		{
			image<rgb_pixel> frame(count_i, count_j);
			
			for (int i = 0; i < count_i; i++)
				for (int j = 0; j < count_j; j++)
					frame[j][i] = frame_base[j][i];
					
			draw_particle(frame, count_i, count_j, item.state, lab_size, axis_1, axis_2);
			
			flip_imabe_v(frame, count_i, count_j);
			
			frame.write((output_dir / fs::path((bo::format("labmap_%s%s_t%u.png") % get_axis(axis_1) % get_axis(axis_2) % t).str())).string());
			
			t++;
		}
	}
}

void draw_node(image<rgb_pixel> frame_base, Simulation& simulation, SimluationResultNodeSummary& summary_node, LabSizePlane& lab_size, LabMapLimit& limit, int count_i, int count_j, short axis_1, short axis_2, double dt, long unsigned int& t, Pulse& laser, FunctionFieldType function_field, fs::path output_dir)
{
	double last_time  = summary_node.items.front().local_time;

	for (SimluationResultNodeItem& item: summary_node.items)
	{
		double current_time = item.local_time - last_time;
		
		if (current_time >= dt)
		{
			image<rgb_pixel> frame(count_i, count_j);
			
			// Copying backgrounds (borders)
			for (int i = 0; i < count_i; i++)
				for (int j = 0; j < count_j; j++)
					frame[j][i] = frame_base[j][i];	
					
			// Drawing fields
			draw_field(frame, count_i, count_j, item, summary_node.node, limit, lab_size, axis_1, axis_2, simulation, laser, function_field);
				
			// Drawing particle
			ParticleStateGlobal state_global;
			state_local_to_global(state_global, item.local_state, summary_node.node);
			draw_particle(frame, count_i, count_j, state_global, lab_size, axis_1, axis_2);
			
			flip_imabe_v(frame, count_i, count_j);
			
			frame.write((output_dir / fs::path((bo::format("labmap_%s%s_t%u.png") % get_axis(axis_1) % get_axis(axis_2) % t).str())).string());
			
			t++;
			last_time += dt;
		}
	}
}


void get_field_limits(Simulation& simulation, Pulse& laser, LabSizePlane& lab_size, LabMapLimit& limit, vector<SimluationResultNodeSummary>& summaries_node, short axis_1, short axis_2, FunctionFieldType function_field)
{
	limit.e_mod_min = +INFINITY;
	limit.b_mod_min = +INFINITY;
	
	limit.e_mod_max = -INFINITY;
	limit.b_mod_max = -INFINITY;
	
	int radius = trunc(simulation.laser_influence_radius / lab_size.dr);
	
	for (SimluationResultNodeSummary& summary_node: summaries_node)
	{
		double last_time  = summary_node.items.front().local_time;
		
		for (SimluationResultNodeItem& item: summary_node.items)
		{
			
			double current_time = item.local_time - last_time;
		
			if (current_time >= simulation.time_resolution_free)
			{
				for (int i = -radius + 1; i < radius - 1; i++)
				{
					for (int j = -radius + 1; j < radius - 1; j++)
					{
						if (i * i + j * j < radius * radius)
						{
							Field field;
							
							get_node_field(field, summary_node.node, laser, lab_size, i, j, axis_1, axis_2, item.local_time, function_field);
							
							double e_mod = vector_module(field.e_x, field.e_y, field.e_z);
							double b_mod = vector_module(field.b_x, field.b_y, field.b_z);
							
							if (e_mod < limit.e_mod_min) limit.e_mod_min = e_mod;
							if (e_mod > limit.e_mod_max) limit.e_mod_max = e_mod;
							if (b_mod < limit.b_mod_min) limit.b_mod_min = b_mod;
							if (b_mod > limit.b_mod_max) limit.b_mod_max = b_mod;
						}
					}
				}
				
				last_time += simulation.time_resolution_free;
			}
		}
	}
}


void render_labmap(Laboratory& laboratory, Simulation& simulation, Pulse& laser, vector<SimluationResultFreeSummary>& summaries_free, vector<SimluationResultNodeSummary>& summaries_node, short axis_1, short axis_2, FunctionFieldType function_field, fs::path output_dir)
{
	
	LabSizeGlobal lab_size_global;
	compute_lab_limits(laboratory, simulation, summaries_free, lab_size_global);
	
	LabSizePlane lab_size = get_lab_size(lab_size_global, axis_1, axis_2);

	int count_i = trunc(lab_size.size_1 / lab_size.dr);
	int count_j = trunc(lab_size.size_2 / lab_size.dr);
	
	
	image<rgb_pixel> frame_base(count_i, count_j);
	draw_base(frame_base, count_i, count_j, simulation, laboratory, lab_size, axis_1, axis_2);
	frame_base.write((output_dir / fs::path((bo::format("labmap_%s%s_base.png") % get_axis(axis_1) % get_axis(axis_2)).str())).string());

	LabMapLimit limit;
	get_field_limits(simulation, laser, lab_size, limit, summaries_node, axis_1, axis_2, function_field);

	unsigned long t = 0;
	
	
	unsigned int f = 0;
	unsigned int n = 0;
	
	// We take care that we draw the electron inside the laser pulse with the same time rate of the electron in free movement
	while (f < summaries_free.size() || n < summaries_node.size())
	{
		if (f < summaries_free.size() && n < summaries_node.size())
		{
			if (summaries_free[f].time_enter < summaries_node[n].global_time_offset + summaries_node[n].local_time_enter)
			{
				draw_free(frame_base, summaries_free[f], lab_size, count_i, count_j, axis_1, axis_2, t, simulation, lab_size_global, output_dir);
				f++;
			}
			else
			{
				draw_node(frame_base, simulation, summaries_node[n], lab_size, limit, count_i, count_j, axis_1, axis_2, simulation.time_resolution_free, t, laser, function_field, output_dir);
				n++;
			}
		}
		else if (f < summaries_free.size())
		{
			draw_free(frame_base, summaries_free[f], lab_size, count_i, count_j, axis_1, axis_2, t, simulation, lab_size_global, output_dir);
			f++;
		}
		else if (n < summaries_node.size())
		{
			draw_node(frame_base, simulation, summaries_node[n], lab_size, limit, count_i, count_j, axis_1, axis_2, simulation.time_resolution_free, t, laser, function_field, output_dir);
			n++;
		}
	}
	
	
	
	plot_labmap(output_dir, axis_1, axis_2);
}

