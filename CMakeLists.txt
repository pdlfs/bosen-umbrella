#
# CMakeLists.txt  build bosen and its environment
# 27-Sep-2017  chuck@ece.cmu.edu
#              gamvrosi@cs.cmu.edu
#

#
# general command line config:
#
#   -DCMAKE_INSTALL_PREFIX=/usr/local      # installation prefix
#   -DCMAKE_BUILD_TYPE=RelWithDebInfo      # or Release, Debug, etc.
#      (XXX: currently only applied to cmake-based builds)
#
#   -DUMBRELLA_BUILD_TESTS=ON              # build unit tests?
#   -DUMBRELLA_SKIP_TESTS=OFF              # skip running unit tests?
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
# be specified using a semicolon to create a path).
#
# specifying alternate compilers (overrides CC/CXX environment variables):
# -DCC=/bin/cc
# -DCXX=/bin/cxx
#  (you can also use the long form names CMAKE_C_COMPILER/CMAKE_CXX_COMPILER)
#
# specifying which mpi to use by pointing at the wrappers:
# -DMPI_C_COMPILER=/usr/bin/mpicc
# -DMPI_CXX_COMPILER=/usr/bin/mpicxx
# -DMPIEXEC=/usr/bin/mpiexec
#

cmake_minimum_required (VERSION 3.0)

#
# put the umbrella scripts in our search path and include umbrella-init
# (umbrella-init needs to run before "project")
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
###umbrella_opt_default (BOOST_TAG "436ad1d")        # 1.65.1
umbrella_opt_default (GFLAGS_TAG "46f73f8")       # v2.2.1
umbrella_opt_default (GLOG_TAG "2a6df662")        # Oct 2017
umbrella_opt_default (GOOGLETEST_TAG "ec44c6c")   # 1.8.0
umbrella_opt_default (GPERFTOOLS_TAG "bf840de")   # 2.6.1
umbrella_opt_default (LIBZMQ_TAG "8d75d50")       # 4.2.2
umbrella_opt_default (SNAPPY_TAG "b02bfa7")       # 1.1.7
umbrella_opt_default (SPARSEHASH_TAG "6af4bcf")   # 2.0.3

#
# other options
#
umbrella_opt_default (GPERFTOOLS_FRAMEPOINTER "on")

###include (umbrella/boost)
include (umbrella/gflags)
include (umbrella/glog)
include (umbrella/googletest)
include (umbrella/gperftools)
include (umbrella/libzmq)
include (umbrella/snappy)
include (umbrella/sparsehash)
