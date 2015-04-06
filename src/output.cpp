#include <stdio.h>
#include "response.hpp"
#include "util.hpp"
#include "type.hpp"

extern string ffmpeg_name;

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


void setup_response_analysis(ofstream& stream, ResponseAnalysis& response_analysis)
{
	stream.setf(ios::scientific);
	stream.precision(16);
	
	stream
		<< "#"
		<< (bo::format("in_%s_%s_perc (%%)")   % response_analysis.object_in  % response_analysis.attribute_in ).str()	<< ";"
		<< (bo::format("in_%s_%s_delta (%s)")  % response_analysis.object_in  % response_analysis.attribute_in  % get_conversion_si_unit(response_analysis.object_in,  response_analysis.attribute_in )).str() << ";" 
		<< (bo::format("in_%s_%s_abs (%s)")    % response_analysis.object_in  % response_analysis.attribute_in  % get_conversion_si_unit(response_analysis.object_in,  response_analysis.attribute_in )).str() << ";" ;
	
	for (unsigned int o = 0; o < response_analysis.attribute_out.size(); o++)
	{
		string object_out    = response_analysis.object_out[o];
		string attribute_out = response_analysis.attribute_out[o];
		
		stream
			<< (bo::format("out_%s_%s_perc (%%)")  % object_out % attribute_out).str()	<< ";"
			<< (bo::format("out_%s_%s_delta (%s)") % object_out % attribute_out % get_conversion_si_unit(object_out, attribute_out)).str() <<  ";"
			<< (bo::format("out_%s_%s_abs (%s)")   % object_out % attribute_out % get_conversion_si_unit(object_out, attribute_out)).str() <<  ";";
	}
	stream << endl;
}


void write_response_analysis(ofstream& stream, ResponseAnalysis& response_analysis, double perc_in,  double delta_in, double value_in, vector<double> perc_out, vector<double> delta_out, vector<double> value_out)
{
	double unit_in  = get_conversion_si_value(response_analysis.object_in,  response_analysis.attribute_in);
	
	
	stream
		<< perc_in				<< ";"
		<< delta_in * unit_in 	<< ";" 
		<< value_in * unit_in 	<< ";";
		
	for (unsigned int o = 0; o < response_analysis.attribute_out.size(); o++)
	{
		double unit_out = get_conversion_si_value(response_analysis.object_out[o], response_analysis.attribute_out[o]);
		
		stream
			<< perc_out[o]				<< ";"
			<< delta_out[o] * unit_out	<< ";"
			<< value_out[o] * unit_out	<< ";";
	}
	
	stream << endl;
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


void write_particle(ofstream& stream, double current_time, ParticleStateGlobal& state)
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
	ParticleStateLocal& particle_state,
	Field& field)
{
	
	double rel_pos_rho;
	double rel_pos_theta;
	double rel_pos_phi;
	
	double rel_mom_rho;
	double rel_mom_theta;
	double rel_mom_phi;
	
	
	rel_pos_rho = vector_module(particle_state.position_x, particle_state.position_y, particle_state.position_z);
	cartesian_to_spherical(particle_state.position_x, particle_state.position_y, particle_state.position_z, rel_pos_theta, rel_pos_phi);
	
	rel_mom_rho = vector_module(particle_state.momentum_x, particle_state.momentum_y, particle_state.momentum_z);
	cartesian_to_spherical(particle_state.momentum_x, particle_state.momentum_y, particle_state.momentum_z, rel_mom_theta, rel_mom_phi);
	
	stream
		<< current_time	* AU_TIME							<< ";"
		<< particle_state.position_x    * AU_LENGTH 		<< ";" 
		<< particle_state.position_y    * AU_LENGTH 		<< ";" 
		<< particle_state.position_z    * AU_LENGTH 		<< ";" 
		<< rel_pos_rho  				* AU_LENGTH 		<< ";" 
		<< rel_pos_theta 									<< ";" 
		<< rel_pos_phi			 							<< ";" 
		<< particle_state.momentum_x 	* AU_MOMENTUM		<< ";" 
		<< particle_state.momentum_y 	* AU_MOMENTUM		<< ";" 
		<< particle_state.momentum_z 	* AU_MOMENTUM		<< ";" 
		<< rel_mom_rho 					* AU_MOMENTUM		<< ";" 
		<< rel_mom_theta									<< ";" 
		<< rel_mom_phi 										<< ";"
		<< field.e_x					* AU_ELECTRIC_FIELD	<< ";"
		<< field.e_y 					* AU_ELECTRIC_FIELD	<< ";"
		<< field.e_z 					* AU_ELECTRIC_FIELD	<< ";"
		<< field.b_x 					* AU_MAGNETIC_FIELD	<< ";"
		<< field.b_y 					* AU_MAGNETIC_FIELD	<< ";"
		<< field.b_z 					* AU_MAGNETIC_FIELD	<< ";"
		<< rel_mom_phi 										<< endl;
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

void save_field_render_data(FieldRenderResult& field_render_result, FieldRenderData& field_render_data, fs::path output_dir)
{
	
	unsigned int t  = field_render_data.t;
	unsigned int na = field_render_result.na;
	unsigned int nb = field_render_result.nb;

	FieldRender& field_render = field_render_result.render;

	FILE* file_csv=fopen((output_dir / fs::path((bo::format("field_render_%s_t%u.csv") % field_render.id % t).str())).string().c_str(), "w");

	fprintf(file_csv, (bo::format("#time;%s;%s") % field_render_result.axis1_label % field_render_result.axis2_label).str().c_str());
	for (unsigned short c = 0; c < field_render.count; c++)
		fprintf(file_csv, (bo::format(";value_%u") % c).str().c_str());
	fprintf(file_csv, "\n");

	double time = field_render_result.time_start + t * (field_render_result.time_end - field_render_result.time_start);

	for (unsigned int a = 0; a < na; a++)
	{
		double pos_a = -field_render_result.length_a/2 + field_render_result.length_a / na * a;
		for (unsigned int b = 0; b < nb; b++)
		{
			double pos_b = -field_render_result.length_b/2 + field_render_result.length_b / nb * b;
			fprintf(file_csv, "%.16E;%.16E;%.16E", time * AU_TIME, pos_a * AU_LENGTH, pos_b * AU_LENGTH);
			
			for (unsigned short c = 0; c < field_render.count; c++)
				fprintf(file_csv, ";%.16E", field_render_data.values[a][b][c]);
				
			fprintf(file_csv, "\n");
		}
	}

	fclose(file_csv);
	
}


void save_field_render_ct2(FieldRenderResult& field_render_result, fs::path output_dir)
{
	FieldRender& field_render = field_render_result.render;
	
	for (unsigned short c = 0; c < field_render.count; c++)
	{
		string basename_global =  (bo::format("field_render_%s_%u") % field_render.id % c).str();
		
		fs::path filename_ct = output_dir / fs::path((bo::format("%s.ct2") % basename_global).str());
		
		FILE* file_ct = fopen(filename_ct.string().c_str(), "w");
		
		// Writing ctioga2 files
		fprintf(file_ct, "title '%s'\n", field_render.titles[c].c_str());
		fprintf(file_ct, "text-separator ;\n");
		fprintf(file_ct, "\n");
		fprintf(file_ct, "z_min = %.16E\n", 0.0); // TODO: add value
		fprintf(file_ct, "z_max = %.16E\n", 100.0); // TODO: add value
		fprintf(file_ct, "\n");
		fprintf(file_ct, "xyz-map\n");
		fprintf(file_ct, "new-zaxis zvalues /location right /bar_size=6mm\n");
		
		FieldRenderResultLimit& render_limit = field_render_result.limits[c];
		string color_range = field_render.colors[c];
		bo::replace_all(color_range, "min_abs", (bo::format("%E") % render_limit.value_min_abs).str());
		bo::replace_all(color_range, "max_abs", (bo::format("%E") % render_limit.value_max_abs).str());
		bo::replace_all(color_range, "min", 	(bo::format("%E") % render_limit.value_min).str());
		bo::replace_all(color_range, "max", 	(bo::format("%E") % render_limit.value_max).str());

		
		fprintf(file_ct, "plot @'$2:$3:$%u'  /color-map \"%s\" /zaxis zvalues\n", 4+c, color_range.c_str());
		fprintf(file_ct, "\n");
		fprintf(file_ct, "xlabel '$%s$ [$m$]'\n", field_render_result.axis1_label.c_str());
		fprintf(file_ct, "ylabel '$%s$ [$m$]'\n", field_render_result.axis2_label.c_str());
		
		fclose(file_ct);
	}
}


void save_field_render_sh(FieldRenderResult& field_render_result, fs::path output_dir)
{
	FieldRender& field_render = field_render_result.render;
	
	for (unsigned short c = 0; c < field_render.count; c++)
	{
		string basename_global =  (bo::format("field_render_%s_%u") % field_render.id % c).str();

		fs::path filename_ct = output_dir / fs::path((bo::format("%s.ct2") % basename_global).str());
		
		// Writing shell file that will create the movie
		fs::path filename_sh = output_dir / fs::path((bo::format("%s.sh") % basename_global).str());
		FILE* file_sh = fopen(filename_sh.string().c_str(), "w");
		fprintf(file_sh, "#!/bin/sh\n");
		fprintf(file_sh, "\n");
		
		fprintf(file_sh, "echo \"Running ctioga2 ...\"\n");
		for (unsigned int t = 0; t < field_render_result.nt; t++)
		{
			string basename_time = (bo::format("%s_t%u") % basename_global % t).str();
			fprintf(file_sh, "ctioga2 --no-mark --text-separator \\; --load 'field_render_%s_t%u.csv' -f '%s'  --name '%s'\n", field_render.id.c_str() , t, filename_ct.filename().string().c_str(), basename_time.c_str());
		}
		fprintf(file_sh, "\n");
		
		fprintf(file_sh, "echo \"Running pdftoppm ...\"\n");
		for (unsigned int t = 0; t < field_render_result.nt; t++)
		{
			string basename_time = (bo::format("%s_t%u") % basename_global % t).str();
			fprintf(file_sh, (bo::format("pdftoppm -png -scale-to 1080 -singlefile '%s.pdf' '%s'\n") % basename_time % basename_time).str().c_str());
		}
		fprintf(file_sh, "\n");
		fprintf(file_sh, (bo::format("rm %s_t*.pdf\n") % basename_global).str().c_str());
		fprintf(file_sh, "\n");



		
		double framerate = ceil(field_render_result.nt / (field_render.movie_length * AU_TIME));
		
		fprintf(file_sh, "echo \"Running %s ...\"\n", ffmpeg_name.c_str());
		fprintf(file_sh, "%s -framerate %.5f -loglevel error -i '%s_t%%d.png' -c:v libx264 -r 30 '%s.mp4'", ffmpeg_name.c_str(), framerate, basename_global.c_str(),/* w, h,*/ basename_global.c_str());
		fprintf(file_sh, "\n");
		fprintf(file_sh, (bo::format("rm %s_t*.png\n") % basename_global).str().c_str());
		fprintf(file_sh, "\n");
		fprintf(file_sh, "echo \"done\"\n");
		fclose(file_sh);
		
		system((bo::format("chmod a+x %s") % filename_sh.string()).str().c_str());
	}
}

void save_response_analysis_ct2(ResponseAnalysis& response_analysis, fs::path output_dir)
{
	string object_in     = response_analysis.object_in;
	string attribute_in  = response_analysis.attribute_in;
	
	for (unsigned int o = 0; o < response_analysis.attribute_out.size(); o++)
	{
		string object_out    = response_analysis.object_out[o];
		string attribute_out = response_analysis.attribute_out[o];
		
		unsigned int offset = (1+o) * 3;
		//------------------------------------------------------------------
		ofstream s1;
		s1.open((output_dir / fs::path((bo::format("response_%s_%s_perc.ct2") % object_out % attribute_out).str())).string());
		
		string xlabel2 = (bo::format("in %s %s [\\%%]")  % object_in  % attribute_in).str();
		string ylabel2 = (bo::format("out %s %s [\\%%]") % object_out % attribute_out).str();
		bo::replace_all(xlabel2, "_", " ");
		bo::replace_all(ylabel2, "_", " ");
		
		s1 << bo::format("title 'Response perc %u'") % response_analysis.id << endl;
		s1 << bo::format("name 'response_%s_%s_perc'") % object_out % attribute_out << endl;
		s1 << bo::format("xlabel '%s'") % xlabel2 << endl;
		s1 << bo::format("ylabel '%s'") % ylabel2 << endl;
		s1 << "marker bullet" << endl;
		s1 << "line-style no" << endl;
		s1 << "plot @$1*100:$" << offset + 1 << "*100" << endl;
		s1.close();
		
		//------------------------------------------------------------------
		ofstream s2;
		s2.open((output_dir / fs::path((bo::format("response_%s_%s_delta.ct2") % object_out % attribute_out).str())).string());
		
		string xlabel1 = (bo::format("in %s %s [%s]")  % object_in  % attribute_in  % get_conversion_si_unit(object_in,  attribute_in )).str();
		string ylabel1 = (bo::format("out %s %s [%s]") % object_out % attribute_out % get_conversion_si_unit(object_out, attribute_out)).str();
		bo::replace_all(xlabel1, "_", " ");
		bo::replace_all(ylabel1, "_", " ");
		
		s2 << bo::format("title 'Response delta %u'") % response_analysis.id << endl;
		s2 << bo::format("name 'response_%s_%s_delta'") % object_out % attribute_out << endl;
		s2 << bo::format("xlabel '%s'") % xlabel1 << endl;
		s2 << bo::format("ylabel '%s'") % ylabel1 << endl;
		s2 << "marker bullet" << endl;
		s2 << "marker-scale 0.15" << endl;
		s2 << "line-style no" << endl;
		s2 << "plot @2:" << offset + 2 << endl;
		s2.close();
		
		//------------------------------------------------------------------
		ofstream s3;
		s3.open((output_dir / fs::path((bo::format("response_%s_%s_abs.ct2") % object_out % attribute_out).str())).string());
		
		string xlabel3 = (bo::format("in %s %s [%s]")  % object_in  % attribute_in  % get_conversion_si_unit(object_in,  attribute_in )).str();
		string ylabel3 = (bo::format("out %s %s [%s]") % object_out % attribute_out % get_conversion_si_unit(object_out, attribute_out)).str();
		bo::replace_all(xlabel3, "_", " ");
		bo::replace_all(ylabel3, "_", " ");
		
		s3 << bo::format("title 'Response abs %u'") % response_analysis.id << endl;
		s3 << bo::format("name 'response_%s_%s_abs'") % object_out % attribute_out << endl;
		s3 << bo::format("xlabel '%s'") % xlabel3 << endl;
		s3 << bo::format("ylabel '%s'") % ylabel3 << endl;
		s3 << "marker bullet" << endl;
		s3 << "line-style no" << endl;
		s3 << "plot @3:" << offset + 3 << endl;
		s3.close();
	}
}

void save_response_analysis_sh(ResponseAnalysis& response_analysis, fs::path output_dir)
{
	fs::path f = (output_dir / fs::path("response.sh"));
	ofstream s;
	s.open(f.string());
	
	s << "#!/bin/sh" << endl << endl;
	s << "echo \"Running ctioga2 ...\"" << endl;
	
	for (unsigned int o = 0; o < response_analysis.attribute_out.size(); o++)
	{
		string object_out    = response_analysis.object_out[o];
		string attribute_out = response_analysis.attribute_out[o];
		
		s << bo::format("ctioga2 --no-mark --text-separator \\; --load 'response.csv' -f 'response_%s_%s_delta.ct2'") % object_out % attribute_out << endl;
		s << bo::format("ctioga2 --no-mark --text-separator \\; --load 'response.csv' -f 'response_%s_%s_perc.ct2'" ) % object_out % attribute_out << endl;
		s << bo::format("ctioga2 --no-mark --text-separator \\; --load 'response.csv' -f 'response_%s_%s_abs.ct2'"  ) % object_out % attribute_out << endl;
	}
	
	system((bo::format("chmod a+x %s") % f.string()).str().c_str());	
}
