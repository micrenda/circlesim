

void perform_render(
	fs::path base_dir,
	fs::path interaction_base_path,
	fs::path render_file_path,
	fs::path subrender_file_path,
	unsigned int i,
	string       r,
	unsigned int s);
{
	
}


int main(int argc, char *argv[])
{
	
	fs::path base_dir 		= fs::path("");
	
	
	vector<unsigned int> 	arg_interactions;
	vector<string>	arg_renders;
	vector<unsigned int>		arg_subrenders;

	int flag;
	static struct option long_options[] = {
		{"help",   		    0, 0, 'h'},
		{"interaction",	    1, 0, 'i'},
		{"render",	    1, 0, 'r'},
		{"subrender",	1, 0, 's'},
		{NULL, 0, NULL, 0}
	};
	
	int option_index = 0;
	while ((flag = getopt_long(argc, argv, "hi:r:s:", long_options, &option_index)) != -1)
	{
		switch (flag)
		{
		case 'i':
			arg_interactions.push_back(stoi(optarg));
		break;
		case 'r':
			arg_renders.push_back(optarg);
		break;
		case 's':
			arg_subrenders.push_back(stoi(optarg));
		break;
		case 'o':
			output_dir = fs::path(optarg);
		break;
		case 'h':
			print_help();
			exit(0);
		break;
		case '?':
			print_help();
			exit(-1);
		break;
		default:
			printf ("?? getopt returned character code 0%o ??\n", flag);
			exit(-1);
		break;
		}
	}
	

	
	if (optind < argc)
    {
     
		if (optind == argc - 1)
			base_dir = fs::path(argv[optind++]);
		else
		{
			printf("Only one base directory can be provided\n");
			exit(-1);
		}
    }
	
	if (base_dir == fs::path(""))
	{
		printf("Please specify the base directory.\n");

		exit(-1);
	}
	
	if (!fs::is_directory(base_dir))
	{
		printf("Directory '%s' does not exists\n", base_dir.c_str());
		exit(-1);
	}
	
		
		
	// Getting current node
	static const bo::regex regex_interaction("^i([0-9]+)n([0-9]+)$");
	
	vector<unsigned int> 	selected_interactions;
	
	if (arg_interactions.empty())
	{
		fs::directory_iterator end_iter;
		for( fs::directory_iterator dir_iter(base_dir) ; dir_iter != end_iter ; ++dir_iter)
		{
			if (fs::is_directory(dir_iter->status()))
			{
				std::string dir_name = std::string(dir_iter->path().filename().string());
				
				bo::match_results<std::string::const_iterator> what;
				if (bo::regex_match(dir_name, what, regex_interaction))
				{
					selected_interactions.push_back(stoi(what[0]))
				}
			}
		}
	}
	else
	{
		for (unsigned int i : arg_interactions)
		{
			selected_interactions.push_back(i);
		}
	}
	
	
	
	
	
	for (unsigned int i : arg_interactions)
	{
		fs::path interaction_base_path;
		
		// Finding the interaction folder
		fs::directory_iterator end_iter;
		for( fs::directory_iterator dir_iter(base_dir) ; dir_iter != end_iter ; ++dir_iter)
		{
			if (fs::is_directory(dir_iter->status()))
			{
				std::string dir_name = std::string(dir_iter->path().filename().string());
				
				bo::match_results<std::string::const_iterator> what;
				if (bo::regex_match(dir_name, what, e))
				{
					if (stoi(what[1]) == current_interaction)
					{
						interaction_base_path = dir_iter->path();
					}
				}
			}
		}
		
		if (interaction_base_path != fs::path())
		{
			printf("Unable to find interaction %d in directory '%s'.\n", i, base_dir.c_str());
			exit(-1);
		}
		
		
		// Ok, we have the interaction folder, now we start to look for render 
		static const bo::regex_render("^field_render_([[:print:]]+)\\.cfg$");
		vector<string>	selected_renders;
		
		if (arg_render_bases.empty())
		{
			fs::path render_path;
			// Finding the interaction folder
			fs::directory_iterator end_iter;
			for( fs::directory_iterator file_iter(interaction_base_path) ; file_iter != end_iter ; ++dir_iter)
			{
				if (fs::is_regular_file(file_iter->status()))
				{
					std::string file_name = std::string(file_iter->path().filename().string());
					
					bo::match_results<std::string::const_iterator> what;
					if (bo::regex_match(dir_name, what, regex_render))
					{
						selected_renders.push_back(what[0]);
					}
				}
			}
		}
		else
		{
			for(string& r: arg_renders)
				selected_renders.push_back(r);
		}
			
		
		
		for (string& r: selected_renders)
		{
			fs::path render_file_path = interaction_base_path / fs::path((bo::format("field_render_%s.cfg") % r).str());
			
			if (!fs::is_regular_file(render_file_path->status()))
			{
				printf("File '%s' does not exists\n", render_file_path.c_str());
				exit(-1);
			}
			
			
			// Now we have a verified interaction and render, let search for subrender
			static const bo::regex_subrender("^field_render_([[:print:]]+)_r([0-9]+).dat$");
			vector<unsigned int> selected_subrenders;
			
			if (arg_subrenders.empty())
			{
				fs::directory_iterator end_iter;
				for( fs::directory_iterator file_iter(interaction_base_path) ; file_iter != end_iter ; ++dir_iter)
				{
					if (fs::is_regular_file(file_iter->status()))
					{
						std::string file_name = std::string(file_iter->path().filename().string());
						
						bo::match_results<std::string::const_iterator> what;
						if (bo::regex_match(dir_name, what, regex_render))
						{
							if (what[0] == r)
								selected_renders.push_back(stoi(what[1]));
						}
					}
				}
			}
			else
			{
				for (unsigned int s : arg_subrenders)
					selected_subrenders.push_back(s);
			}
			
			
			for (unsigned int s: selected_subrenders)
			{
				
				fs::path subrender_file_path = interaction_base_path / fs::path((bo::format("field_render_%s_r%i.dat") % r % s).str());
			
				if (!fs::is_regular_file(subrender_file_path->status()))
				{
					printf("File '%s' does not exists\n", subrender_file_path.c_str());
					exit(-1);
				}			
				
				
				
				
				// Ok, let render our file
				perform_render(base_dir, interaction_base_path, render_file_path, subrender_file_path);
				
					
			}
		}	
	}
}
