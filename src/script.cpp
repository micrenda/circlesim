#include <stdio.h>
#include "script.hpp"
void build_auxiliary_library(vector<string>& headers, vector<string>& sources, fs::path output_dir)
{
	
	fs::path filename_cpp = output_dir / fs::path("custom_scripts.cpp");
	fs::path filename_hpp = output_dir / fs::path("custom_scripts.hpp");
	fs::path filename_so  = output_dir / fs::path("custom_scripts.so");
	
	// Writing cpp
	
	ofstream cpp;
	cpp.open (filename_cpp.string());
	
	cpp << "#include <math.h>" 				<< endl;
	cpp << "#include \"custom_scripts.hpp\""<< endl;
	cpp 									<< endl;
	cpp										<< endl;

	for (string source: sources)
	{
		cpp << source				<< endl;
	}

	cpp.close();
	
	// Writing hpp
	
	ofstream hpp;
	hpp.open (filename_hpp.string());
	
	hpp << "using namespace std;" 	<< endl;
	hpp << "" 						<< endl;
	hpp << "#include <vector>" 		<< endl;
	hpp << "#include <map>" 		<< endl;
	hpp << "" 						<< endl;
	hpp << "" 						<< endl;
	hpp << "typedef struct Field" 	<< endl;
	hpp << "{"						<< endl;
	hpp << "	double e_x;"		<< endl;
	hpp << "	double e_y;"		<< endl;
	hpp << "	double e_z;"		<< endl;
	hpp << ""						<< endl;
	hpp << "	double b_x;"		<< endl;
	hpp << "	double b_y;"		<< endl;
	hpp << "	double b_z;"		<< endl;
	hpp << "} Field;"				<< endl;
	hpp << ""						<< endl;
	hpp << "typedef struct PulseParams"				<< endl;
	hpp << "{"										<< endl;
	hpp << "	// These are the only parameter that are not converted to A.U. (the main rule is that all values inside the program must be converted to A.U.)" << endl;
	hpp << "	map<string, int>	params_int;"	<< endl;
	hpp << "	map<string, long>	params_int64;"	<< endl;
	hpp << "	map<string, double> params_float;"	<< endl;
	hpp << "	map<string, string> params_string;"	<< endl;
	hpp << "	map<string, bool>	params_boolean;"	<< endl;
	hpp << "} PulseParams;"	<< endl;
	
	for (string header: headers)
	{
		hpp << header				<< endl;
	}
	hpp.close();
	
	// building
	string cmd = (bo::format("g++ -std=c++11 -g -Wall -shared -fPIC -o %s %s -lm") % filename_so.string() % filename_cpp.string()).str();
	
	cout << bo::format("Building '%s' with cmd:") % filename_so.filename().string() << endl;
	cout << "-----------------------------------------------------------" << endl;
	cout << cmd << endl;
	cout << "-----------------------------------------------------------" << endl;
	
	int result = system(cmd.c_str());
	
	if (result != 0)
	{
		cout << bo::format("ERROR - There was a problem while building %s library. Please check the output above.") % filename_cpp.string() << endl;
		exit(-5);
	}
}
