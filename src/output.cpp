#include <stdio.h>
#include "util.hpp"
#include "type.hpp"

void setup_particle(ofstream& stream)
{
	stream.setf(ios::scientific);
	stream.precision(16);
	stream
		<< "#"
		<< "time" 					<< ";" 
		<< "position_x" 			<< ";" 
		<< "position_y" 			<< ";"
		<< "position_z" 			<< ";"
		<< "momentum_x" 			<< ";"
		<< "momentum_y" 			<< ";"
		<< "momentum_z" 			<< endl;
}

void setup_node(ofstream& stream)
{
	stream.setf(ios::scientific);
	stream.precision(16);
	stream 
		<< "#"
		<< "id" 					<< ";" 
		<< "position_x" 			<< ";" 
		<< "position_y" 			<< ";"
		<< "position_z" 			<< ";"
		<< "rot_1_1" 				<< ";"
		<< "rot_1_2" 				<< ";"
		<< "rot_1_3" 				<< ";"
		<< "rot_2_1" 				<< ";"
		<< "rot_2_2" 				<< ";"
		<< "rot_2_3" 				<< ";"
		<< "rot_3_1" 				<< ";"
		<< "rot_3_2" 				<< ";"
		<< "rot_3_3" 				<< endl;
}

void setup_interaction(ofstream& stream)
{
	stream.setf(ios::scientific);
	stream.precision(16);
	stream 
		<< "#"
		<< "time"					<< ";"
		<< "relative_position_x" 	<< ";" 
		<< "relative_position_y" 	<< ";"
		<< "relative_position_z" 	<< ";"
		<< "relative_position_rho" 	<< ";" 
		<< "relative_position_theta"<< ";"
		<< "relative_position_phi" 	<< ";"
		<< "relative_momentum_x" 	<< ";"
		<< "relative_momentum_y" 	<< ";"
		<< "relative_momentum_z" 	<< ";"
		<< "relative_momentum_rho" 	<< ";"
		<< "relative_momentum_theta"<< ";"
		<< "relative_momentum_phi" 	<< ";"
		<< "field_e_x"			 	<< ";"
		<< "field_e_y"			 	<< ";"
		<< "field_e_z"			 	<< ";"
		<< "field_b_x"			 	<< ";"
		<< "field_b_y"			 	<< ";"
		<< "field_b_z"			 	<< endl;
}

void setup_particle_field(ofstream& stream)
{
	stream.setf(ios::scientific);
	stream.precision(16);
	stream 
		<< "#"
		<< "time"					<< ";"
		<< "relative_position_x" 	<< ";" 
		<< "relative_position_y" 	<< ";"
		<< "relative_position_z" 	<< ";"
		<< "field_e_x"			 	<< ";"
		<< "field_e_y"			 	<< ";"
		<< "field_e_z"			 	<< ";"
		<< "field_b_x"			 	<< ";"
		<< "field_b_y"			 	<< ";"
		<< "field_b_z"			 	<< endl;
}

void setup_field_map_xy(ofstream& stream)
{
	stream.setf(ios::scientific);
	stream.precision(16);
	stream 
		<< "#"
		<< "time"					<< ";"
		<< "relative_position_x"	<< ";"
		<< "relative_position_y"	<< ";"
		<< "relative_position_z"	<< ";"
		<< "field_e_x"			 	<< ";"
		<< "field_e_y"			 	<< ";"
		<< "field_e_z"			 	<< ";"
		<< "field_b_x"			 	<< ";"
		<< "field_b_y"			 	<< ";"
		<< "field_b_z"			 	<< endl;
}


void setup_field_map_xz(ofstream& stream)
{	
	stream.setf(ios::scientific);	
	stream.precision(16);
	stream
		<< "#"
		<< "time"					<< ";"
		<< "relative_position_x"	<< ";"
		<< "relative_position_y"	<< ";"
		<< "relative_position_z"	<< ";"
		<< "field_e_x"			 	<< ";"
		<< "field_e_y"			 	<< ";"
		<< "field_e_z"			 	<< ";"
		<< "field_b_x"			 	<< ";"
		<< "field_b_y"			 	<< ";"
		<< "field_b_z"			 	<< endl;
}

void setup_field_map_yz(ofstream& stream)
{
	stream.setf(ios::scientific);
	stream.precision(16);
	stream
		<< "#"
		<< "time"					<< ";"
		<< "relative_position_x"	<< ";"
		<< "relative_position_y"	<< ";"
		<< "relative_position_z"	<< ";"
		<< "field_e_x"			 	<< ";"
		<< "field_e_y"			 	<< ";"
		<< "field_e_z"			 	<< ";"
		<< "field_b_x"			 	<< ";"
		<< "field_b_y"			 	<< ";"
		<< "field_b_z"			 	<< endl;	
}


string get_filename_particle(fs::path output_dir)
{
	return (output_dir / fs::path("particle.csv")).string();
}

string get_filename_node(fs::path output_dir)
{
	return (output_dir / fs::path("node.csv")).string();
}

string get_filename_interaction(fs::path output_dir)
{
	return (output_dir / fs::path("interaction.csv")).string();
}

string get_filename_particle_field(fs::path output_dir)
{
	return (output_dir / fs::path("particle_field.csv")).string();
}


void write_particle(ofstream& stream, double current_time, ParticleState& state)
{
	stream.precision(16);
	stream 
		<< current_time		* AU_TIME		<< ";";
		
	stream.precision(20);
	stream 
		<< state.position_x	* AU_LENGTH		<< ";" 
		<< state.position_y	* AU_LENGTH		<< ";" 
		<< state.position_z	* AU_LENGTH		<< ";";
		
	stream.precision(16);
	stream  
		<< state.momentum_x * AU_MOMENTUM	<< ";"
		<< state.momentum_y	* AU_MOMENTUM	<< ";"
		<< state.momentum_z * AU_MOMENTUM	<< endl;
}

void write_interaction(
	ofstream& stream, 
	double current_time, 
	double rel_pos_x, 
	double rel_pos_y, 
	double rel_pos_z, 
	double rel_mom_x, 
	double rel_mom_y, 
	double rel_mom_z,
	Field& field)
{
	
	double rel_pos_rho;
	double rel_pos_theta;
	double rel_pos_phi;
	
	double rel_mom_rho;
	double rel_mom_theta;
	double rel_mom_phi;
	
	
	rel_pos_rho = vector_module(rel_pos_x, rel_pos_y, rel_pos_z);
	cartesian_to_spherical(rel_pos_x, rel_pos_y, rel_pos_z, rel_pos_theta, rel_pos_phi);
	
	rel_mom_rho = vector_module(rel_mom_x, rel_mom_y, rel_mom_z);
	cartesian_to_spherical(rel_mom_x, rel_mom_y, rel_mom_z, rel_mom_theta, rel_mom_phi);
	
	stream
		<< current_time	* AU_TIME			<< ";"
		<< rel_pos_x    * AU_LENGTH 		<< ";" 
		<< rel_pos_y    * AU_LENGTH 		<< ";" 
		<< rel_pos_z    * AU_LENGTH 		<< ";" 
		<< rel_pos_rho  * AU_LENGTH 		<< ";" 
		<< rel_pos_theta 					<< ";" 
		<< rel_pos_phi			 			<< ";" 
		<< rel_mom_x 	* AU_MOMENTUM		<< ";" 
		<< rel_mom_y 	* AU_MOMENTUM		<< ";" 
		<< rel_mom_z 	* AU_MOMENTUM		<< ";" 
		<< rel_mom_rho 	* AU_MOMENTUM		<< ";" 
		<< rel_mom_theta					<< ";" 
		<< rel_mom_phi 						<< ";"
		<< field.e_x	* AU_ELECTRIC_FIELD	<< ";"
		<< field.e_y 	* AU_ELECTRIC_FIELD	<< ";"
		<< field.e_z 	* AU_ELECTRIC_FIELD	<< ";"
		<< field.b_x 	* AU_MAGNETIC_FIELD	<< ";"
		<< field.b_y 	* AU_MAGNETIC_FIELD	<< ";"
		<< field.b_z 	* AU_MAGNETIC_FIELD	<< ";"
		<< rel_mom_phi 						<< endl;
}

void write_particle_field(
	ofstream& stream, 
	double current_time, 
	double rel_pos_x, 
	double rel_pos_y, 
	double rel_pos_z, 
	Field& field)
{
	stream
		<< current_time	* AU_TIME			<< ";"
		<< rel_pos_x    * AU_LENGTH 		<< ";" 
		<< rel_pos_y    * AU_LENGTH 		<< ";" 
		<< rel_pos_z    * AU_LENGTH 		<< ";" 
		<< field.e_x	* AU_ELECTRIC_FIELD	<< ";"
		<< field.e_y 	* AU_ELECTRIC_FIELD	<< ";"
		<< field.e_z 	* AU_ELECTRIC_FIELD	<< ";"
		<< field.b_x 	* AU_MAGNETIC_FIELD	<< ";"
		<< field.b_y 	* AU_MAGNETIC_FIELD	<< ";"
		<< field.b_z 	* AU_MAGNETIC_FIELD	<< endl;
}

void write_node(ofstream& stream, Node& node)
{
	stream 
		<< node.id 						<< ";" 
		<< node.position_x * AU_LENGTH	<< ";" 
		<< node.position_y * AU_LENGTH	<< ";" 
		<< node.position_z * AU_LENGTH	<< ";" 
		<< node.axis(0,0)				<< ";"
		<< node.axis(0,1)				<< ";"
		<< node.axis(0,2)				<< ";"
		<< node.axis(1,0)				<< ";"
		<< node.axis(1,1)				<< ";"
		<< node.axis(1,2)				<< ";"
		<< node.axis(2,0)				<< ";"
		<< node.axis(2,1)				<< ";"
		<< node.axis(2,2)				<< endl;
}

void write_field_maps_xy(ofstream& stream, double time, double position_x, double position_y, double position_z, Field& field)
{
	stream 
		<< time			* AU_TIME			<< ";" 
		<< position_x	* AU_LENGTH			<< ";" 
		<< position_y	* AU_LENGTH			<< ";" 
		<< position_z	* AU_LENGTH			<< ";" 
		<< field.e_x	* AU_ELECTRIC_FIELD	<< ";" 
		<< field.e_y	* AU_ELECTRIC_FIELD	<< ";" 
		<< field.e_z	* AU_ELECTRIC_FIELD	<< ";" 
		<< field.b_x	* AU_MAGNETIC_FIELD	<< ";" 
		<< field.b_y	* AU_MAGNETIC_FIELD	<< ";"
		<< field.b_z	* AU_MAGNETIC_FIELD	<< endl;
}

void write_field_maps_xz(ofstream& stream, double time, double position_x, double position_y, double position_z, Field& field)
{
	stream 
		<< time			* AU_TIME			<< ";" 
		<< position_x	* AU_LENGTH			<< ";" 
		<< position_y	* AU_LENGTH			<< ";" 
		<< position_z	* AU_LENGTH			<< ";" 
		<< field.e_x	* AU_ELECTRIC_FIELD	<< ";" 
		<< field.e_y	* AU_ELECTRIC_FIELD	<< ";" 
		<< field.e_z	* AU_ELECTRIC_FIELD	<< ";" 
		<< field.b_x	* AU_MAGNETIC_FIELD	<< ";" 
		<< field.b_y	* AU_MAGNETIC_FIELD	<< ";"
		<< field.b_z	* AU_MAGNETIC_FIELD	<< endl;
}

void write_field_maps_yz(ofstream& stream, double time, double position_x, double position_y, double position_z, Field& field)
{
	stream 
		<< time			* AU_TIME			<< ";" 
		<< position_x	* AU_LENGTH			<< ";" 
		<< position_y	* AU_LENGTH			<< ";" 
		<< position_z	* AU_LENGTH			<< ";" 
		<< field.e_x	* AU_ELECTRIC_FIELD	<< ";" 
		<< field.e_y	* AU_ELECTRIC_FIELD	<< ";" 
		<< field.e_z	* AU_ELECTRIC_FIELD	<< ";" 
		<< field.b_x	* AU_MAGNETIC_FIELD	<< ";" 
		<< field.b_y	* AU_MAGNETIC_FIELD	<< ";"
		<< field.b_z	* AU_MAGNETIC_FIELD	<< endl;
}



void export_field_render(
	unsigned int nt,
	unsigned int n_a,
	unsigned int n_b,
	double   start_t,
	double   end_t,
	double   len_a,
	double   len_b,
	FieldRender& field_render,
	double**** space,
	string axis1,
	string axis2,
	fs::path output_dir)
{
	for (unsigned int t = 0; t < nt; t++)
	{
		FILE* file_csv=fopen((output_dir / fs::path((bo::format("field_render_%s_t%u.csv") % field_render.id % t).str())).string().c_str(), "w");
	
		fprintf(file_csv, (bo::format("#time;%s;%s") % axis1 % axis2).str().c_str());
		for (unsigned short c = 0; c < field_render.count; c++)
			fprintf(file_csv, (bo::format(";value_%u") % c).str().c_str());
		fprintf(file_csv, "\n");
	
		double time = start_t + t * field_render.time_resolution;
		
		for (unsigned int a = 0; a < n_a; a++)
		{
			double pos_a = -len_a/2 + field_render.space_resolution * a;
			for (unsigned int b = 0; b < n_b; b++)
			{
				double pos_b = -len_b/2 + field_render.space_resolution * b;
				fprintf(file_csv, "%.16E;%.16E;%.16E", time * AU_TIME, pos_a * AU_LENGTH, pos_b * AU_LENGTH);
				
				for (unsigned short c = 0; c < field_render.count; c++)
					fprintf(file_csv, ";%.16E", space[t][a][b][c]);
					
				fprintf(file_csv, "\n");
			}
		}
		
		fclose(file_csv);
	}
}
