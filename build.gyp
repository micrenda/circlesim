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
				'src/util.cpp'
			],
			
			'include_dirs': ['util/LuaState/include/'],
			
			
			'conditions':
			[
				[
					'OS=="linux"',
					{
						'cflags': 		['-std=c++11', '-Wall', '-fopenmp'],
						'include_dirs':
						[
							'/usr/include/lua5.1/',
						],
						
					}
				],
				
				[
					'LINKING=="static"',
					{
						'libraries':	
						[
							'-static-libgcc',
							'-static-libstdc++',
							'-Wl,-Bstatic',
							'-fopenmp', '-lgsl', '-lgslcblas', '-lconfig++', '-lboost_filesystem', '-lboost_system', '-lboost_regex', 
							'-Wl,-Bdynamic',
							'-llua5.1', '-larmadillo'
						],
					},
					{
						'libraries'	  : ['-fopenmp', '-lgsl', '-lgslcblas', '-lconfig++', '-lboost_filesystem', '-lboost_system', '-lboost_regex', '-llua5.1', '-larmadillo'],
					}
				],
				[
					'TARGET=="debug"',
					{
						'cflags': 		['-O0 -g']
					},
					{
						'cflags': 		['-O3'],
					}
					
				]
			]
		}
	]
}
