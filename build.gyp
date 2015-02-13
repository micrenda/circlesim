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
			'libraries'	  : ['-fopenmp', '-lgsl', '-lgslcblas', '-lconfig++', '-lboost_filesystem', '-lboost_system', '-lboost_regex', '-llua5.2', '-larmadillo'],
			
			'conditions':
			[
				[
					'OS=="linux"',
					{
						'cflags': 		['-std=c++11', '-Wall', '-fopenmp'],
						'include_dirs':
						[
							'/usr/include/',
							'/usr/include/lua5.2/',
						],
						
					}
				],
				[
					'LINKING=="static"',
					{
						'libraries':	
						[
							'-static',
							'-static-libstdc++',
						],
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
