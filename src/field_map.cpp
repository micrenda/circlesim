#include <math.h>
#include <iostream>
#include <fstream>
#include "output.hpp"
#include "plot.hpp"
#include "field_map.hpp"

void update_limits(FieldRenderResultLimit& limit, double value)
{
	if (value < limit.value_min) limit.value_min = value;
	if (value > limit.value_max) limit.value_max = value;
	double value_abs = abs(value);
	if (value_abs < limit.value_min_abs) limit.value_min_abs = value_abs;
	if (value_abs > limit.value_max_abs) limit.value_max_abs = value_abs;
}

void calculate_field_map(FieldRenderResult& field_render_result, FieldRender& field_render, unsigned int interaction, int node,  Pulse& laser, FunctionFieldType function_field, fs::path output_dir)
{
	
	
		
	// Initializing field_render_result
	
	field_render_result.node 		= node;
	field_render_result.interaction	= interaction;
	
	field_render_result.render = field_render;
	


	field_render_result.time_start = field_render.time_start;
	field_render_result.time_end   = field_render.time_end;
	
	double& start_t = field_render_result.time_start;
	double& end_t   = field_render_result.time_end;
	

	unsigned int nt = (end_t - start_t) / field_render.time_resolution;
	unsigned int ni = field_render.space_size_x / field_render.space_resolution;
	unsigned int nj = field_render.space_size_y / field_render.space_resolution;
	unsigned int nk = field_render.space_size_z / field_render.space_resolution;
	
	field_render_result.nt = nt;
	
	
	vector<ofstream*> bindata_files;
	
	for (unsigned int r = 0; r < field_render.count; r++)
	{
		ofstream* file =new ofstream((output_dir / fs::path((bo::format("field_render_%s_r%u.dat") % field_render.id % r).str())).string().c_str(), ios::binary);
		bindata_files.push_back(file);
	}
	
	
	
	

	for (unsigned short c = 0; c < field_render.count; c++)
	{
		FieldRenderResultLimit render_limit;
		render_limit.value_min = +INFINITY;
		render_limit.value_max = -INFINITY;
		render_limit.value_min_abs = +INFINITY;
		render_limit.value_max_abs = 0;
		
		field_render_result.limits.push_back(render_limit);
	}

	
	
	// Creating space arrays with the field for the entire duration
	switch(field_render.plane)
	{
		case XY:
			field_render_result.na = ni;
			field_render_result.nb = nj;
			field_render_result.length_a = field_render.space_size_x;
			field_render_result.length_b = field_render.space_size_y;
			
			save_field_render_cfg (field_render_result,       output_dir);
			
			#pragma omp parallel for ordered schedule(static, 1)
			for (unsigned int t = 0; t < nt; t++)
			{
				double time = start_t + t * field_render.time_resolution;
				
				FieldRenderData data;
				data.t = t;
				data.values = new double**[ni];
				
				
				for (unsigned int i = 0; i < ni; i++)
				{
					double x = -field_render.space_size_x/2 + field_render.space_resolution * i;
					data.values[i] = new double*[nj];
					for (unsigned int j = 0; j < nj; j++)
					{
						double y = -field_render.space_size_y/2 + field_render.space_resolution * j;
						data.values[i][j] = new double[field_render.count];
						
						vector<double> values = field_render.function_render(time * AU_TIME, x * AU_LENGTH, y * AU_LENGTH, field_render.axis_cut * AU_LENGTH);
						
						for (unsigned short c = 0; c < field_render.count; c++)
						{
							data.values[i][j][c] = values[c];
							update_limits(field_render_result.limits[c], values[c]);
						}
					}
				}
				
				//save_field_render_data(field_render_result, data, output_dir);
				
				
				#pragma omp ordered
				
				write_field_render_bindata(bindata_files, field_render_result, data);	
				
				// Freeing the allocated memory
				for (unsigned int s2 = 0; s2 < field_render_result.na; s2++)
				{
					for (unsigned int s3 = 0; s3 < field_render_result.nb; s3++)
					{
						delete [] data.values[s2][s3];
					}
					delete [] data.values[s2];
				}
				delete []  data.values;
				
				printf("\rRendering %s: %.3f%%", field_render.id.c_str(), 100.d * (t+1) / nt);
				fflush(stdout);
			}
		break;
		
		case XZ:
			field_render_result.na = ni;
			field_render_result.nb = nk;
			field_render_result.length_a = field_render.space_size_x;
			field_render_result.length_b = field_render.space_size_z;
		
			save_field_render_cfg (field_render_result,       output_dir);
			
			#pragma omp parallel for ordered schedule(static, 1)
			for (unsigned int t = 0; t < nt; t++)
			{
				double time = start_t + t * field_render.time_resolution;
				
				FieldRenderData data;
				data.t = t;
				data.values = new double**[ni];
				
				for (unsigned int i = 0; i < ni; i++)
				{
					double x = -field_render.space_size_x/2 + field_render.space_resolution * i;
					data.values[i] = new double*[nk];
					for (unsigned int k = 0; k < nk; k++)
					{
						double z = -field_render.space_size_z/2 + field_render.space_resolution * k;
						data.values[i][k] = new double[field_render.count];
						
						vector<double> values = field_render.function_render(time * AU_TIME, x * AU_LENGTH, field_render.axis_cut * AU_LENGTH, z * AU_LENGTH);
						
						for (unsigned short c = 0; c < field_render.count; c++)
						{
							data.values[i][k][c] = values[c];
							update_limits(field_render_result.limits[c], values[c]);
						}
					}
				}
				
				//save_field_render_data(field_render_result, data, output_dir);
				
				#pragma omp ordered
				
				write_field_render_bindata(bindata_files, field_render_result, data);	
				
				// Freeing the allocated memory
				for (unsigned int s2 = 0; s2 < field_render_result.na; s2++)
				{
					for (unsigned int s3 = 0; s3 < field_render_result.nb; s3++)
					{
						delete [] data.values[s2][s3];
					}
					delete [] data.values[s2];
				}
				delete [] data.values;
				
				printf("\rRendering %s: %.3f%%", field_render.id.c_str(), 100.d * (t+1) / nt);
				fflush(stdout);
			}
		break;
		
		case YZ:
			field_render_result.na = nj;
			field_render_result.nb = nk;
			field_render_result.length_a = field_render.space_size_y;
			field_render_result.length_b = field_render.space_size_z;
			
			save_field_render_cfg (field_render_result,       output_dir);
			
			#pragma omp parallel for ordered schedule(static, 1)
			for (unsigned int t = 0; t < nt; t++)
			{
				double time = start_t + t * field_render.time_resolution;
				
				FieldRenderData data;
				data.t = t;
				data.values = new double**[nj];
				
				
				for (unsigned int j = 0; j < nj; j++)
				{
					double y = -field_render.space_size_y/2 + field_render.space_resolution * j;
					data.values[j] = new double*[nk];
					for (unsigned int k = 0; k < nk; k++)
					{
						double z = -field_render.space_size_z/2 + field_render.space_resolution * k;
						data.values[j][k] = new double[field_render.count];
						
						vector<double> values = field_render.function_render(time * AU_TIME, field_render.axis_cut * AU_LENGTH, y * AU_LENGTH, z * AU_LENGTH);
						
						for (unsigned short c = 0; c < field_render.count; c++)
						{
							data.values[j][k][c] = values[c];
							update_limits(field_render_result.limits[c], values[c]);
						}
					}
				}
				
				//save_field_render_data(field_render_result, data, output_dir);
				
				#pragma omp ordered
				
				write_field_render_bindata(bindata_files, field_render_result, data);	
				
				// Freeing the allocated memory
				for (unsigned int s2 = 0; s2 < field_render_result.na; s2++)
				{
					for (unsigned int s3 = 0; s3 < field_render_result.nb; s3++)
					{
						delete [] data.values[s2][s3];
					}
					delete [] data.values[s2];
				}
				delete [] data.values;
				
				printf("\rRendering %s: %.3f%%", field_render.id.c_str(), 100.d * (t+1) / nt);
				fflush(stdout);
			}
		break;
		
	}
	
	//
	for (unsigned int r = 0; r < field_render.count; r++)
	{
		bindata_files[r]->close();
	}
	
	
	
	// Creating files needed to create the video
	save_field_render_ct2(field_render_result, output_dir);
	save_field_render_sh(field_render_result, output_dir);
	
	printf("\n");
}	
