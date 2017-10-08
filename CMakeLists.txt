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

option (UMBRELLA_BUILD_TESTS OFF)   # turn it off

#
# pull in the main routines
#
include (umbrella-main)

#
# set git tags for modules
#
umbrella_opt_default (EIGEN_TAG "5a0156e40feb")      # 3.3.4
umbrella_opt_default (GFLAGS_TAG "46f73f8")          # v2.2.1
umbrella_opt_default (GLOG_TAG "2a6df662")           # Oct 2017
umbrella_opt_default (GOOGLETEST_TAG "ec44c6c")      # 1.8.0
umbrella_opt_default (GPERFTOOLS_TAG "bf840de")      # 2.6.1
umbrella_opt_default (LEVELDB_TAG "a53934a")         # 1.20
umbrella_opt_default (LIBCONFIG_TAG "5a06a16")       # v1.6
umbrella_opt_default (LIBCUCKOO_TAG "57129c8c")      # Oct 2017
umbrella_opt_default (OPROFILE_TAG "RELEASE_1_2_0")  # 1.2.0
umbrella_opt_default (SNAPPY_TAG "b02bfa7")          # 1.1.7
umbrella_opt_default (SPARSEHASH_TAG "6af4bcf")      # 2.0.3
umbrella_opt_default (YAML_CPP_TAG "b57efe9 ")       # 0.5.3

#
# other options
#
umbrella_opt_default (GPERFTOOLS_FRAMEPOINTER "on")

include (umbrella/boost)        # its 1.65.1
include (umbrella/eigen)
include (umbrella/gflags)
include (umbrella/glog)
include (umbrella/googletest)
include (umbrella/gperftools)
include (umbrella/leveldb)
include (umbrella/libconfig)
include (umbrella/libcuckoo)
include (umbrella/libzmq3x)     # default is 3.2.5
include (umbrella/oprofile)
include (umbrella/popt)         # its 1.16 from a tar file
include (umbrella/snappy)
include (umbrella/sparsehash)
include (umbrella/yaml-cpp)

#
# other packages
#
ExternalProject_Add (float16_compressor
    DOWNLOAD_COMMAND true    # no need to download, comes with umbrella
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/misc/float16_compressor
    CMAKE_CACHE_ARGS ${UMBRELLA_CMAKECACHE}

    UPDATE_COMMAND ""
)
ExternalProject_Add (zmqhpp DEPENDS libzmq3x
    DOWNLOAD_COMMAND true    # no need to download, comes with umbrella
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/misc/zmqhpp
    CMAKE_CACHE_ARGS ${UMBRELLA_CMAKECACHE}

    UPDATE_COMMAND ""
)
