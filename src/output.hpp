#include "type.hpp"

void open_global_files(fs::path output_dir);
void open_interaction_files(fs::path output_dir, unsigned int interaction, int node);
void open_field_map_xy_files(fs::path output_dir, unsigned int interaction, int node, unsigned int t);
void open_field_map_xz_files(fs::path output_dir, unsigned int interaction, int node, unsigned int t);
void open_field_map_yz_files(fs::path output_dir, unsigned int interaction, int node, unsigned int t);

void write_node(Node& node);
void write_particle(double current_time, ParticleState& state);
void write_interaction(double time_current,	double rel_pos_x, double rel_pos_y, double rel_pos_z, double rel_mom_x, double rel_mom_y, double rel_mom_z,	FieldEB& field);
void write_field(double current_time, double rel_pos_x,  double rel_pos_y,  double rel_pos_z,  FieldEB& field);

void write_field_maps_xy(double time, double position_x, double position_y, double position_z, FieldEB& field);
void write_field_maps_xz(double time, double position_x, double position_y, double position_z, FieldEB& field);
void write_field_maps_yz(double time, double position_x, double position_y, double position_z, FieldEB& field);

void close_interaction_files();
void close_global_files();
void close_field_map_xy_files();
void close_field_map_xz_files();
void close_field_map_yz_files();
