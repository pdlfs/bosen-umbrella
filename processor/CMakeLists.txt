cmake_minimum_required(VERSION 3.0)
project (bigml-processor CXX)

find_package (MPI MODULE REQUIRED)
string (REPLACE " " ";" MPI_CXX_COMPILE_FLAGS_LIST "${MPI_CXX_COMPILE_FLAGS}")

# link shared lib with full rpath
set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable (bigml-processor bigml-processor.cxx)
add_executable (bigml-verifier bigml-verifier.cxx)

foreach (lcv ${MPI_CXX_COMPILE_FLAGS_LIST})
    if (NOT ${lcv} STREQUAL "")
        target_compile_options (bigml-processor PUBLIC ${lcv})
        target_compile_options (bigml-verifier PUBLIC ${lcv})
    endif ()
endforeach ()

# XXX: have to do this one dir at a time otherwise, otherwise I get
# error: target 'deltafs' INTERFACE_INCLUDE_DIRECTORIES contains path
#               prefixed in the source directory
foreach (lcv ${MPI_CXX_INCLUDE_PATH})
    target_include_directories (bigml-processor PUBLIC ${lcv})
    target_include_directories (bigml-verifier PUBLIC ${lcv})
endforeach ()

foreach (lcv ${MPI_CXX_LIBRARIES})
    target_link_libraries (bigml-processor ${lcv})
    target_link_libraries (bigml-verifier ${lcv})
endforeach ()

set_property (TARGET bigml-processor APPEND PROPERTY LINK_FLAGS
              ${MPI_CXX_LINK_FLAGS})
set_property (TARGET bigml-verifier APPEND PROPERTY LINK_FLAGS
              ${MPI_CXX_LINK_FLAGS})

# executable scripts to install
set (install-scripts common.sh)

# executable scripts to @-expand and then install
set (install-scripts-in process_data.sh.in)

#
# expand ${install-scripts-in} and build a list of output files for install
#
foreach (lcv ${install-scripts-in})
    string (REGEX REPLACE "\\.in$" "" out "${lcv}")
    configure_file (${lcv} ${out} @ONLY)
    list (APPEND install-scripts-in-done ${CMAKE_CURRENT_BINARY_DIR}/${out})
endforeach ()

#
# "make install" rules
#
install (TARGETS bigml-processor RUNTIME DESTINATION bin)
install (TARGETS bigml-verifier RUNTIME DESTINATION bin)
install (PROGRAMS ${install-scripts} DESTINATION processor)
install (PROGRAMS ${install-scripts-in-done} DESTINATION processor)
