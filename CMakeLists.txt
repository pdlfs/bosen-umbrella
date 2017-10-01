#
# CMakeLists.txt  build bosen and its environment
# 27-Sep-2017  chuck@ece.cmu.edu
#              gamvrosi@cs.cmu.edu
#

# command line config:
#
# -DCMAKE_INSTALL_PREFIX=/tmp/delta       # where to install
#                                         # defaults to /usr/local
#

#
# [optional config vars]
#
# -DCMAKE_BUILD_TYPE=RelWithDebInfo       # or Release, Debug, etc.
#     (this only gets reflected to cmake-based external builds,
#      it isn't applied to autotools-based builds...)
# -DSKIP_TESTS=OFF                    # skip running unit tests
#
# finding dependencies:
#
# -DCMAKE_PREFIX_PATH='/pkg'              # look for additional installs here
#
# the following also applies for configure scripts:
# -DCMAKE_INCLUDE_PATH='/pkg/include'     # extra include directories
# -DCMAKE_LIBRARY_PATH='/pkg/lib'         # extra library path
#
# note these are all cmake lists (so more than one directory can
# be specified).
#
# specifying alternate compilers (overrides CC/CXX environment variables):
# -DCC=/bin/cc
# -DCXX=/bin/cxx
#  (you can also use the long form names CMAKE_C_COMPILER/CMAKE_CXX_COMPILER)
#
cmake_minimum_required (VERSION 3.1)

#
# put the umbrella scripts in our search path and include umbrella-init
# (configuration that needs to take place prior to declaring "project")
#
list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/umbrella")
include (umbrella-init)
project (bosen-umbrella C CXX)

#
# pull in the main routines
#
include (umbrella-main)

#
# set git tags for modules
#
umbrella_opt_default (GFLAGS_TAG "46f73f8")

include (modules/gflags)
