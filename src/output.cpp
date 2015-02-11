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

string get_filename_interaction(fs::path output_dir, unsigned int interaction, int node)
{
	return (output_dir / fs::path((bo::format("interaction_i%un%d.csv") %  interaction % node).str())).string();
}

string get_filename_particle_field(fs::path output_dir, unsigned int interaction, int node)
{
	return (output_dir / fs::path((bo::format("particle_field_i%un%d.csv") %  interaction % node).str())).string();
}

string get_filename_field_map_xy(fs::path output_dir, unsigned int interaction, int node, unsigned int t)
{
	return ((output_dir / fs::path((bo::format("field_map_xy_i%un%dt%u.csv") %  interaction % node % t).str())).c_str());
}

string get_filename_field_map_xz(fs::path output_dir, unsigned int interaction, int node, unsigned int t)
{	
	return (output_dir / fs::path((bo::format("field_map_xz_i%un%dt%u.csv") %  interaction % node % t).str())).string();
}

string get_filename_field_map_yz(fs::path output_dir, unsigned int interaction, int node, unsigned int t)
{
	return (output_dir / fs::path((bo::format("field_map_yz_i%un%dt%u.csv") %  interaction % node % t).str())).string();
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
	FieldEB& field)
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
	FieldEB& field)
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

void write_field_maps_xy(ofstream& stream, double time, double position_x, double position_y, double position_z, FieldEB& field)
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

void write_field_maps_xz(ofstream& stream, double time, double position_x, double position_y, double position_z, FieldEB& field)
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

void write_field_maps_yz(ofstream& stream, double time, double position_x, double position_y, double position_z, FieldEB& field)
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
