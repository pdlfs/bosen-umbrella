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
set (BOSEN_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}"
     CACHE STRING "bosen install directory")

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

#
# bosen - we install bosen in ${BOSEN_INSTALL_PREFIX} which defaults
# to ${CMAKE_INSTALL_PREFIX} but can be split out into its own directory.
# (the main bosen umbrella is big enough you might want to share it across
# multiple bosen installs).   we also mark it as EXCLUDE_FROM_ALL so it
# it is only built on request...
#

# fold in BOSEN_INSTALL_PREFIX
set (bosen-cmake-cache ${UMBRELLA_CMAKECACHE}
    -DCMAKE_INSTALL_PREFIX:STRING=${BOSEN_INSTALL_PREFIX})

umbrella_download (BOSEN_DOWNLOAD bosen bosen.tar.gz
    GIT_REPOSITORY git@github.com:jinliangwei/parameter_server
    GIT_TAG hpc)
umbrella_patchcheck (BOSEN_PATCHCMD bosen)

ExternalProject_Add (bosen
    DEPENDS boost eigen gflags glog googletest gperftools leveldb libconfig
            libcuckoo libzmq3x oprofile popt snappy sparsehash yaml-cpp
    ${BOSEN_DOWNLOAD} ${BOSEN_PATCHCMD}
    CMAKE_ARGS -DBUILD_SHARED_LIBS=ON
    CMAKE_CACHE_ARGS ${bosen-cmake-cache}
    UPDATE_COMMAND "")
set_property (TARGET bosen PROPERTY EXCLUDE_FROM_ALL True)

# and now the apps we want
ExternalProject_Add (bosen-lda DEPENDS bosen
    DOWNLOAD_COMMAND true    # no need to download, comes with bosen
    SOURCE_DIR ${CMAKE_BINARY_DIR}/bosen-prefix/src/bosen/apps/lda
    CMAKE_CACHE_ARGS ${bosen-cmake-cache}

    UPDATE_COMMAND ""
)
set_property (TARGET bosen-lda PROPERTY EXCLUDE_FROM_ALL True)

ExternalProject_Add (bosen-matrixfact DEPENDS bosen
    DOWNLOAD_COMMAND true    # no need to download, comes with bosen
    SOURCE_DIR ${CMAKE_BINARY_DIR}/bosen-prefix/src/bosen/apps/matrixfact
    CMAKE_CACHE_ARGS ${bosen-cmake-cache}

    UPDATE_COMMAND ""
)
set_property (TARGET bosen-matrixfact PROPERTY EXCLUDE_FROM_ALL True)
