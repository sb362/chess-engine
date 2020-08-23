cmake_minimum_required (VERSION 3.0.0)

add_subdirectory (fmt)

add_library (chess bitboard.cc bitboard.hh kogge-impl.hh magic.cc magic.hh
				   types.hh position.cc position.hh movegen.cc movegen.hh)

add_library (engine perft.cc perft.hh ucioption.cc ucioption.hh uci.cc uci.hh
					moveorder.cc moveorder.hh search.cc search.hh heuristics.hh
					tt.cc tt.hh evaluation.cc evaluation.hh
					threading/thread.cc threading/thread.hh)

add_executable (main main.cc)

target_compile_options (chess PRIVATE ${SHARED_FLAGS})
target_compile_options (engine PRIVATE ${SHARED_FLAGS})
target_compile_options (main PRIVATE ${SHARED_FLAGS})

target_link_libraries (chess PUBLIC fmt::fmt)
target_link_libraries (engine PUBLIC chess)
target_link_libraries (main engine)
