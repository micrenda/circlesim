#include "type.hpp"

void setup_particle			(ofstream& stream);
void setup_node				(ofstream& stream);
void setup_interaction		(ofstream& stream);
void setup_particle_field	(ofstream& stream);
void setup_field_map_xy		(ofstream& stream);
void setup_field_map_xz		(ofstream& stream);
void setup_field_map_yz		(ofstream& stream);

string get_filename_particle		(fs::path output_dir);
string get_filename_node			(fs::path output_dir);
string get_filename_interaction		(fs::path output_dir, unsigned int interaction, int node);
string get_filename_particle_field	(fs::path output_dir, unsigned int interaction, int node);
string get_filename_field_map_xy	(fs::path output_dir, unsigned int interaction, int node, unsigned int t);
string get_filename_field_map_xz	(fs::path output_dir, unsigned int interaction, int node, unsigned int t);
string get_filename_field_map_yz	(fs::path output_dir, unsigned int interaction, int node, unsigned int t);

void write_particle			(ofstream& stream, double current_time, ParticleState& state);
void write_interaction		(ofstream& stream, double current_time, double rel_pos_x, double rel_pos_y, double rel_pos_z, double rel_mom_x, double rel_mom_y, double rel_mom_z, Field& field);
void write_particle_field	(ofstream& stream, double current_time, double rel_pos_x, double rel_pos_y, double rel_pos_z, Field& field);
void write_node				(ofstream& stream, Node& node);
void write_field_maps_xy	(ofstream& stream, double time, double position_x, double position_y, double position_z, Field& field);
void write_field_maps_xz	(ofstream& stream, double time, double position_x, double position_y, double position_z, Field& field);
void write_field_maps_yz	(ofstream& stream, double time, double position_x, double position_y, double position_z, Field& field);
