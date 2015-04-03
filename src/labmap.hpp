
void render_labmap(Laboratory& laboratory, Simulation& simulation, Pulse& laser, vector<SimluationResultFreeSummary>& summaries_free, vector<SimluationResultNodeSummary>& summaries_node, short axis_1, short axis_2, FunctionFieldType function_field, fs::path output_dir);

void plot_labmap(fs::path output_dir, short axis_1, short axis_2);
