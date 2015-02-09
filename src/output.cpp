#include <stdio.h>
#include "util.hpp"
#include "type.hpp"

ofstream stream_particle;
ofstream stream_interaction;
ofstream stream_field;
ofstream stream_node;

ofstream stream_map_xy;
ofstream stream_map_xz;
ofstream stream_map_yz;

void open_global_files(fs::path output_dir)
{
	stream_particle.open			((output_dir / fs::path("particle.csv")).c_str());
	stream_node.open				((output_dir / fs::path("node.csv")).c_str());

	stream_particle.setf(ios::scientific);
	stream_node.setf(ios::scientific);
	
	stream_particle.precision(16);
	stream_node.precision(16);
	
	stream_particle 
		<< "#"
		<< "time" 					<< ";" 
		<< "position_x" 			<< ";" 
		<< "position_y" 			<< ";"
		<< "position_z" 			<< ";"
		<< "momentum_x" 			<< ";"
		<< "momentum_y" 			<< ";"
		<< "momentum_z" 			<< endl;

	stream_node 
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

void open_interaction_files(fs::path output_dir, unsigned int interaction, int node)
{
	stream_interaction.open			((output_dir / fs::path((bo::format("interaction_i%un%d.csv") %  interaction % node).str())).c_str());
	stream_field.open				((output_dir / fs::path((bo::format("field_i%un%d.csv") %  interaction % node).str())).c_str());
	
	stream_interaction.setf(ios::scientific);
	stream_field.setf(ios::scientific);
	
	stream_interaction.precision(16);
	stream_field.precision(16);
	
	stream_interaction 
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
		
	stream_field 
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

void open_field_map_xy_files(fs::path output_dir, unsigned int interaction, int node, unsigned int t)
{
	stream_map_xy.open ((output_dir / fs::path((bo::format("field_map_xy_i%un%dt%u.csv") %  interaction % node % t).str())).c_str());
	stream_map_xy.setf(ios::scientific);
	stream_map_xy.precision(16);
	stream_map_xy 
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


void open_field_map_xz_files(fs::path output_dir, unsigned int interaction, int node, unsigned int t)
{	
	
	stream_map_xz.open ((output_dir / fs::path((bo::format("field_map_xz_i%un%dt%u.csv") %  interaction % node % t).str())).c_str());
	stream_map_xz.setf(ios::scientific);	
	stream_map_xz.precision(16);
	stream_map_xz
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

void open_field_map_yz_files(fs::path output_dir, unsigned int interaction, int node, unsigned int t)
{
	stream_map_yz.open ((output_dir / fs::path((bo::format("field_map_yz_i%un%d%u.csv") %  interaction % node % t).str())).c_str());
	stream_map_yz.setf(ios::scientific);
	stream_map_yz.precision(16);
	stream_map_yz
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


void write_particle(double current_time, ParticleState& state)
{
	stream_particle.precision(16);
	stream_particle 
		<< current_time		* AU_TIME		<< ";";
		
	stream_particle.precision(20);
	stream_particle 
		<< state.position_x	* AU_LENGTH		<< ";" 
		<< state.position_y	* AU_LENGTH		<< ";" 
		<< state.position_z	* AU_LENGTH		<< ";";
		
	stream_particle.precision(16);
	stream_particle  
		<< state.momentum_x * AU_MOMENTUM	<< ";"
		<< state.momentum_y	* AU_MOMENTUM	<< ";"
		<< state.momentum_z * AU_MOMENTUM	<< endl;
		
		stream_particle.flush();
}

void write_interaction(
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
	
	stream_interaction
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
		
		stream_interaction.flush();
}

void write_field(
	double current_time, 
	double rel_pos_x, 
	double rel_pos_y, 
	double rel_pos_z, 
	FieldEB& field)
{
	stream_field
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
		
		stream_field.flush();
}

void write_node(Node& node)
{
	stream_node 
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

void write_field_maps_xy(double time, double position_x, double position_y, double position_z, FieldEB& field)
{
	stream_map_xy 
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

void write_field_maps_xz(double time, double position_x, double position_y, double position_z, FieldEB& field)
{
	stream_map_xz 
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

void write_field_maps_yz(double time, double position_x, double position_y, double position_z, FieldEB& field)
{
	stream_map_yz 
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

void close_interaction_files()
{
	stream_interaction.close();
	stream_field.close();
}

void close_global_files()
{
	stream_particle.close();
	stream_node.close();
}

void close_field_map_xy_files()
{
	stream_map_xy.close();
}

void close_field_map_xz_files()
{
	stream_map_xz.close();
}

void close_field_map_yz_files()
{
	stream_map_yz.close();
}
