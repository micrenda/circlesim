{
	'targets':
	[
		{
			'target_name': 'circlesim',
			'type': 'executable',
			'dependencies': [],
			'defines':		[],
			'sources': 
			[
				'src/config.cpp',
				'src/main.cpp',
				'src/output.cpp',
				'src/plot.cpp',
				'src/simulator.cpp',
				'src/util.cpp',
				'src/response.cpp',
				'src/labmap.cpp',
				'src/script.cpp',
				'src/field_map.cpp'
			],
		
			'libraries': 	['-fopenmp', '-lgsl', '-lgslcblas', '-lconfig++', '-lboost_filesystem', '-lboost_system', '-lboost_regex', '-larmadillo' , '-ldl', '-lpng12'],	
			
			'conditions':
			[
				[
					'OS=="linux"',
					{
						'cflags': 		['-std=c++11', '-Wall', '-fopenmp'],
						'include_dirs':
						[
							'/usr/include/libpng12/',
						],
						
					}
				],
				[
					'TARGET=="debug"',
					{
						'cflags': 		['-O0 -g']
					},
					{
						'cflags': 		['-O2'],
					}
					
				]
			]
		}
	]
}
