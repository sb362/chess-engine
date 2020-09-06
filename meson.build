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

attack_gen = get_option('attack_gen')

use_lsb = get_option('use_lsb')
use_popcnt = get_option('use_popcnt')
use_bmi2 = get_option('use_bmi2')

build_tests = get_option('build_tests')

crazyhouse = get_option('crazyhouse')

cc = meson.get_compiler('cpp')

# Check for 64-bit architecture
if cc.sizeof('void *') != 8
	error('Only 64-bit build is supported')
endif

# Disable dumb warnings
if cc.get_id() == 'gcc'
	add_project_arguments('-Wno-class-memaccess', language : 'cpp')
endif

# Intrinsics
if use_lsb
	add_project_arguments('-DUSE_LSB', language : 'cpp')
endif

if use_popcnt
	add_project_arguments('-DUSE_POPCNT', language : 'cpp')

	if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
		add_project_arguments('-mpopcnt', language : 'cpp')
	endif
endif

if use_bmi2
	add_project_arguments('-DUSE_BMI2', language : 'cpp')

	if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
		add_project_arguments('-mbmi2', language : 'cpp')
	endif
endif

# Other
add_project_arguments('-DUSE_' + attack_gen.to_upper(), language : 'cpp')

if (attack_gen == 'pext' or attack_gen == 'pdep') and not use_bmi2
	warning('PEXT or PEXT+PDEP attack generation selected, but BMI2 not enabled.')
endif

if crazyhouse
	add_project_arguments('-DCRAZYHOUSE', language : 'cpp')
endif

summary(
	{
		'POPCNT' : get_option('use_lsb'),
		'LSB/MSB' : get_option('use_popcnt'),
		'BMI2' : get_option('use_bmi2')	
	},
	section : 'Intrinsics'
)

summary(
	{
		'Attack generation' : get_option('attack_gen'),
		'Build tests' : get_option('build_tests')
	},
	section : 'Other'
)

#############################################################################
## Subprojects / dependencies                                              ##
#############################################################################

fmt_proj = subproject('fmt')
fmt_dep = fmt_proj.get_variable('fmt_dep')

if build_tests
	catch2_proj = subproject('Catch2')
	catch2_dep = catch2_proj.get_variable('catch2_dep')
endif

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

if build_tests
	perft_test_exe = executable(
		'tests',
		sources : ['src/perft.test.cc'],
		dependencies : [fmt_dep, catch2_dep, engine_dep]
	)

	test('perft', perft_test_exe)
endif
