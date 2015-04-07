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
void write_response_analysis(ofstream& stream, ResponseAnalysis& response_analysis, double perc_in,  double delta_in, double value_in, vector<double> perct_out, vector<double> delta_out, vector<double> value_out);

void save_field_render_data(FieldRenderResult& field_render_result, FieldRenderData& field_render_data, fs::path output_dir);

void save_field_render_ct2(FieldRenderResult& field_render_result, fs::path output_dir);
void save_field_render_sh (FieldRenderResult& field_render_result, fs::path output_dir);

void save_response_analysis_ct2(ResponseAnalysis& response_analysis, fs::path output_dir);
void save_response_analysis_sh (ResponseAnalysis& response_analysis, fs::path output_dir);
