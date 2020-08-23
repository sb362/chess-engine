cmake_minimum_required (VERSION 3.0.0)

add_subdirectory (Catch2)

add_executable (tests main.cc perft.cc)
target_link_libraries (tests engine Catch2::Catch2)
target_include_directories (tests PRIVATE "${CMAKE_SOURCE_DIR}/src/")
target_compile_options (tests PRIVATE ${SHARED_FLAGS})

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/tests/Catch2/contrib/")

include (CTest)
include (Catch)

catch_discover_tests (tests)
