project(
	'chess-engine',
	'cpp',
	default_options : [
		'cpp_std=c++17',
		'warning_level=3',
		'werror=true',
		'default_library=static',
		'b_lto=true'
	],
	version : '1.0'
)

#############################################################################
## Configuration                                                           ##
#############################################################################

cc = meson.get_compiler('cpp')

if cc.get_id() == 'gcc'
	add_project_arguments('-Wno-class-memaccess', language : 'cpp')
endif

add_project_arguments('-DUSE_' + get_option('attack_gen').to_upper(), language : 'cpp')

#############################################################################
## Subprojects / dependencies                                              ##
#############################################################################

fmt_proj = subproject('fmt')
fmt_dep = fmt_proj.get_variable('fmt_dep')

catch2_proj = subproject('Catch2')
catch2_dep = catch2_proj.get_variable('catch2_dep')

threads_dep = dependency('threads')

#############################################################################
## Source files                                                            ##
#############################################################################

core_headers = [
	'src/types.hh',
	'src/bitboard.hh',
	'src/kogge-impl.hh',
	'src/magic.hh',
	'src/position.hh',
	'src/movegen.hh',

	'src/util/array.hh',
	'src/util/cmdline.hh',
	'src/util/hashtable.hh',
	'src/util/tree.hh',
	'src/util/assert.hh',
	'src/util/compiler.hh',
	'src/util/maths.hh',
	'src/util/tuple.hh',
	'src/util/bits.hh',
	'src/util/enum.hh',
	'src/util/random.hh'
]

core_sources = [
	'src/bitboard.cc',
	'src/magic.cc',
	'src/position.cc',
	'src/movegen.cc'
]

engine_headers = [
	'src/ucioption.hh',
	'src/uci.hh',
	'src/heuristics.hh',
	'src/moveorder.hh',
	'src/search.hh',
	'src/evaluation.hh',
	'src/pawns.hh',
	'src/tt.hh',
	'src/perft.hh',

	'src/threading/thread.hh'
]

engine_sources = [
	'src/ucioption.cc',
	'src/uci.cc',
	'src/moveorder.cc',
	'src/search.cc',
	'src/evaluation.cc',
	'src/pawns.cc',
	'src/tt.cc',
	'src/perft.cc',

	'src/threading/thread.cc',
]

#############################################################################
## Targets                                                                 ##
#############################################################################

core_inc = include_directories('src')

core_lib = library(
	'core',
	sources : core_sources,
	dependencies : [fmt_dep]
)

core_dep = declare_dependency(
	include_directories : core_inc,
	link_with : core_lib
)

engine_lib = library(
	'engine',
	sources : engine_sources,
	dependencies : [fmt_dep, core_dep, threads_dep]
)

engine_dep = declare_dependency(
	link_with : engine_lib
)

engine_exe = executable(
	'engine',
	sources : engine_sources + ['src/main.cc'],
	dependencies : [fmt_dep, engine_dep]
)

perft_test_exe = executable(
	'tests',
	sources : ['src/perft.test.cc'],
	dependencies : [fmt_dep, catch2_dep, engine_dep]
)

test('perft', perft_test_exe)
