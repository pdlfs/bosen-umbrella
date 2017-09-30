#
# CMakeLists.txt  build bosen and its environment
# 28-Sep-2017  gamvrosi@cs.cmu.edu
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
# if CC/CXX is specified from the command line (vs. environment vars)
# we copy them to CMAKE_{C,CXX}_COMPILER early (before 'project') so
# that we probe the desired compiler.   note that cmake already honors
# ENV{CC}/ENV{CXX} so we don't need to do anything for those vars.
#
if (CC)
    set (CMAKE_C_COMPILER ${CC})
endif ()
if (CXX)
    set (CMAKE_CXX_COMPILER ${CXX})
endif ()

project (bosen-umbrella C CXX)
include (ExternalProject)

#
# set default build type and insert it to cache.  add additional options.
#
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE RelWithDebInfo
         CACHE STRING "Choose the type of build." FORCE)
    set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                  "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif ()
set (SKIP_TESTS "OFF" CACHE BOOL "skip running unit tests")
set (BUILD_TESTS "ON" CACHE BOOL "if go build unit tests")

#
# print the current config so users are aware of the current settings...
#
message (STATUS "Current Umbrella settings:")
message (STATUS "  target OS: ${CMAKE_SYSTEM_NAME} "
                             "${CMAKE_SYSTEM_VERSION}")
message (STATUS "  host OS: ${CMAKE_HOST_SYSTEM_NAME} "
                           "${CMAKE_HOST_SYSTEM_VERSION}")
message (STATUS "  build type: ${CMAKE_BUILD_TYPE}")
message (STATUS "  CXX compiler: ${CMAKE_CXX_COMPILER} "
                  "(${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION})")
message (STATUS "  C compiler: ${CMAKE_C_COMPILER} "
                  "(${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION})")
message (STATUS "  crosscompiling: ${CMAKE_CROSSCOMPILING}")
message (STATUS "  build tests: ${BUILD_TESTS} (skip running: ${SKIP_TESTS})")

#
# GenDownloadConfig (result target local-file remote-args ...)
#
# get file locally if present, otherwise download it using remote-args.
#
function (GenDownloadConfig result target local)
    if (EXISTS "${CMAKE_SOURCE_DIR}/cache/${local}")
        message (STATUS "${target}: using cache (${local})")
        # assume correct, but set URL_MD5 to quiet warning
        file (MD5 "${CMAKE_SOURCE_DIR}/cache/${local}" localmd5)
        set (${result} URL "${CMAKE_SOURCE_DIR}/cache/${local}"
                       URL_MD5 ${localmd5} PARENT_SCOPE)
    else ()
        set (${result} ${ARGN} PARENT_SCOPE)
    endif ()
endfunction ()

#
# MaybeRunTests (result test-args ...)
#
# generate test-args output only if 1) we are not cross compiling (so we
# can avoid trying to run target crosscompiled binaries on the host),
# and 2) skip_tests are not set.
#
function (MaybeRunTests result)
    if (NOT ${CMAKE_CROSSCOMPILING} AND NOT ${SKIP_TESTS})
        set (${result} ${ARGN} PARENT_SCOPE)
    else ()
        set (${result} "" PARENT_SCOPE)
    endif ()
endfunction ()

#
# set up the prefix path for packaged software that we may want to
# link to (e.g. third party libraries).   this will get added to
# the configure command line (for autotools-based projects).
#
# we also want our install prefix to be in the prefix path too (it
# isn't by default).
#
list (APPEND CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
list (REMOVE_DUPLICATES CMAKE_PREFIX_PATH)
foreach (prefix ${CMAKE_PREFIX_PATH})
    list (APPEND CMAKE_INCLUDE_PATH "${prefix}/include")
    list (APPEND CMAKE_LIBRARY_PATH "${prefix}/lib")
endforeach ()
list (REMOVE_DUPLICATES CMAKE_INCLUDE_PATH)
list (REMOVE_DUPLICATES CMAKE_LIBRARY_PATH)

#
# build command-line variable settings for autotools configure scripts.
# we reflect the cmake settings (including prefixes) down to the
# configure script via CPPFLAGS, LDFLAGS, CC, AND CXX.
#
if (CMAKE_INCLUDE_PATH)
  foreach (inc ${CMAKE_INCLUDE_PATH})
      set (cppflags "${cppflags} -I${inc}")
  endforeach ()
  string (SUBSTRING ${cppflags} 1 -1 cppflags)   # remove leading space
  set (cppflags "CPPFLAGS=${cppflags}")
endif ()
if (CMAKE_LIBRARY_PATH)
  foreach (lib ${CMAKE_LIBRARY_PATH})
      set (ldflags "${ldflags} -L${lib}")
  endforeach ()
  string (SUBSTRING ${ldflags} 1 -1 ldflags)
  set (ldflags "LDFLAGS=${ldflags}")
endif ()
# compiler settings, the second one is to force an mpi wrapper based compile.
set (ac_comp CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER})

# some systems have this set, so we need to add to it
if (DEFINED ENV{PKG_CONFIG_PATH})
  set (ac_pkgpath
    "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
else ()
  set (ac_pkgpath "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
endif ()
set (ac_pkgcfgpath "PKG_CONFIG_PATH=${ac_pkgpath}")

#
# setup variables to init the cache of cmake-based projects.   we want
# these values to propagate from the umbrella on down...
#
set (init_cm_cache
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
                -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
                -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}
                -DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}
                -DCMAKE_EXPORT_NO_PACKAGE_REGISTRY:BOOL=1
     )

#
# now we are ready to start building packages
#

##
## ch-placement
##
#GenDownloadConfig (chp_download ch-placement ch-placement-0b706ca06.tar.gz
#    GIT_REPOSITORY "http://xgitlab.cels.anl.gov/codes/ch-placement.git"
#    GIT_TAG 0b706ca06		# Dec 2016
#    ### GIT_TAG master
#)
#
#MaybeRunTests (chp_test
#    TEST_COMMAND make check
#)
#
#ExternalProject_Add (ch-placement
#    ${chp_download}
#    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${ac_comp}
#                      --prefix=${CMAKE_INSTALL_PREFIX}
#                      --enable-shared
#    UPDATE_COMMAND ""
#    ${chp_test}
#)
#
#ExternalProject_Add_Step(ch-placement prepare
#    COMMAND ${CMAKE_SOURCE_DIR}/scripts/ensure-autogen <SOURCE_DIR>/prepare
#    COMMENT "preparing source for configure"
#    DEPENDEES update
#    DEPENDERS configure
#    WORKING_DIRECTORY <SOURCE_DIR>
#)
