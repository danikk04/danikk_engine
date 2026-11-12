project(
	'danikk_engine', 
	'cpp',
    default_options: [
        'cpp_std=c++20',
    ]
)

subdir('source')

danikk_framework_proj = subproject('danikk_framework')

include = [
    include_directories('include'),
    danikk_framework_proj.get_variable('include')
]

cpp_args = [
    '-Wvla'
]

lib = static_library(
    'danikk_engine', 
    danikk_engine_source,
    include_directories : include + include_directories('include_lib'),
    link_args : ['-rdynamic'],
    cpp_args : cpp_args
)

this_dep = declare_dependency(
    link_with : lib,
    include_directories : include
)

deps = [
    danikk_framework_proj.get_variable('dep'),
    dependency('glfw3'),
    dependency('libzip'),
    dependency('assimp'),
    dependency('threads'),
    #dependency('dl'),
    this_dep
]
