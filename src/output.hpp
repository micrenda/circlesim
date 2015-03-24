#include "type.hpp"

void setup_particle			(ofstream& stream);
void setup_node				(ofstream& stream);
void setup_interaction		(ofstream& stream);
void setup_response_analysis(ofstream& stream, ResponseAnalysis& response_analysis);

string get_filename_particle		(fs::path output_dir);
string get_filename_node			(fs::path output_dir);
string get_filename_interaction		(fs::path output_dir);

void write_particle			(ofstream& stream, double current_time, ParticleStateGlobal& state);
void write_interaction		(ofstream& stream, double current_time, ParticleStateLocal&  state, Field& field);
void write_node				(ofstream& stream, Node& node);
void write_response_analysis(ofstream& stream, ResponseAnalysis& response_analysis, double perc_in, double value_in, double perc_out, double value_out);

void export_field_render(FieldRenderResult& field_render_result, fs::path output_dir);
