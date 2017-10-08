#
# CMakeLists.txt  cmake for zmqhpp
# 07-Oct-2017  chuck@ece.cmu.edu
#
# this is a very simple package (just a .h file, no compiling required)
#
#  general cmake flags:
#    -DCMAKE_INSTALL_PREFIX=/usr/local     -- the prefix for installing
#

cmake_minimum_required(VERSION 3.0)
project (zmqhpp)

# pull in package helpers and set config file install dir for find_package
include (CMakePackageConfigHelpers)
set (zmqhpp-pkg-loc "share/cmake/zmqhpp")

#
# setup a version number
#
set (ZMQHPP_VERSION_MAJOR 1)
set (ZMQHPP_VERSION_MINOR 0)
set (ZMQHPP_VERSION_PATCH 0)
set (zmqhpp-vers 
    "${ZMQHPP_VERSION_MAJOR}.${ZMQHPP_VERSION_MINOR}")
set (ZMQHPP_VERSION 
    "${zmqhpp-vers}.${ZMQHPP_VERSION_PATCH}")

#
# make an interface library target and add include dir to properties
#
add_library (zmqhpp INTERFACE)
target_include_directories (zmqhpp
                            INTERFACE $<INSTALL_INTERFACE:include>)

#
# installation stuff (packaging and install commands)
#
write_basic_package_version_file(
    "zmqhpp-config-version.cmake"
    VERSION ${ZMQHPP_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
configure_file (zmqhpp-config.cmake.in 
                zmqhpp-config.cmake @ONLY)

#
# "make install" rules
#
install (TARGETS zmqhpp EXPORT zmqhpp-targets)
install (EXPORT zmqhpp-targets
         DESTINATION ${zmqhpp-pkg-loc}
         FILE "zmqhpp-targets.cmake")
install (FILES zmqhpp.hpp
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/zmqhpp-config.cmake"
         "${CMAKE_CURRENT_BINARY_DIR}/zmqhpp-config-version.cmake"
         DESTINATION ${zmqhpp-pkg-loc} )
