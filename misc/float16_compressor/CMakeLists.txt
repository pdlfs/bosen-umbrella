#
# CMakeLists.txt  cmake for float16_compressor
# 05-Oct-2017  chuck@ece.cmu.edu
#
# this is a very simple package (just a .h file, no compiling required)
#
#  general cmake flags:
#    -DCMAKE_INSTALL_PREFIX=/usr/local     -- the prefix for installing
#

cmake_minimum_required(VERSION 3.0)
project (float16_compressor)

# pull in package helpers and set config file install dir for find_package
include (CMakePackageConfigHelpers)
set (f16-pkg-loc "share/cmake/float16_compressor")

#
# setup a version number
#
set (FLOAT16_COMPRESSOR_VERSION_MAJOR 1)
set (FLOAT16_COMPRESSOR_VERSION_MINOR 0)
set (FLOAT16_COMPRESSOR_VERSION_PATCH 0)
set (f16-vers 
    "${FLOAT16_COMPRESSOR_VERSION_MAJOR}.${FLOAT16_COMPRESSOR_VERSION_MINOR}")
set (FLOAT16_COMPRESSOR_VERSION 
    "${f16-vers}.${FLOAT16_COMPRESSOR_VERSION_PATCH}")

#
# make an interface library target and add include dir to properties
#
add_library (float16_compressor INTERFACE)
target_include_directories (float16_compressor
                            INTERFACE $<INSTALL_INTERFACE:include>)

#
# installation stuff (packaging and install commands)
#
write_basic_package_version_file(
    "float16_compressor-config-version.cmake"
    VERSION ${FLOAT16_COMPRESSOR_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
configure_file (float16_compressor-config.cmake.in 
                float16_compressor-config.cmake @ONLY)

#
# "make install" rules
#
install (TARGETS float16_compressor EXPORT float16_compressor-targets)
install (EXPORT float16_compressor-targets
         DESTINATION ${f16-pkg-loc}
         FILE "float16_compressor-targets.cmake")
install (FILES float16_compressor.hpp
         DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/float16_compressor-config.cmake"
         "${CMAKE_CURRENT_BINARY_DIR}/float16_compressor-config-version.cmake"
         DESTINATION ${f16-pkg-loc} )
