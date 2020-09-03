cmake_minimum_required (VERSION 3.0.0)
project (abc VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
	set (SHARED_FLAGS /W3 /WX)
else ()
	set (SHARED_FLAGS -Wall -Wextra -Wdouble-promotion -Wfloat-equal
					  -Wcast-qual -Wcast-align -Wold-style-cast
					  -m64 -msse4 -mpopcnt)
endif ()

# God I hate cmake so much
# todo: ditch this and try Meson
#option (USE_BMI2	"Enable BMI2 extensions" OFF)
#option (USE_LSB		"Use MSB/LSB intrinsics" OFF)
#option (USE_POPCNT	"Use POPCNT intrinsics"  OFF)

#option (ATTACK_GEN  "Sliding piece attack generation (FANCY, KOGGE_STONE, PEXT, PDEP)" "FANCY")

#if (NOT (ATTACK_GEN MATCHES "^(FANCY|KOGGE_STONE|PEXT|PDEP)$"))
#	message (STATUS "${ATTACK_GEN}")
#	message (FATAL_ERROR "Unrecognised option for ATTACK_GEN (FANCY, KOGGE_STONE, PEXT, PDEP only)")
#endif ()

#if (NOT USE_BMI2 AND (ATTACK_GEN EQUAL PEXT OR ATTACK_GEN EQUAL PDEP))
#	message (WARNING "PEXT or PEXT/PDEP attack generation specified, but no BMI2 support.")
#endif ()

add_subdirectory (src)
add_subdirectory (tests)
