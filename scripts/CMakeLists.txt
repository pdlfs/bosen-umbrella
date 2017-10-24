#
# CMakeLists.txt  bosen-umbrella scripts
# 24-Oct-2017  chuck@ece.cmu.edu


# command line config:
#
# -DCMAKE_INSTALL_PREFIX=/tmp/bosen       # where to install
#                                         # defaults to /usr/local
#

cmake_minimum_required (VERSION 3.0)
project (bosen-umbrella-scripts NONE)

# data files to install  (non-executable)
###set (install-files  )

# executable scripts to install
###set (install-programs )

# executable scripts to @-expand and then install
set (install-programs-in run_data_split.sh.in run_matrixfact_split.sh.in)

#
# expand ${install-programs-in} and build a list of output files for install
#
foreach (lcv ${install-programs-in})
    string (REGEX REPLACE "\\.in$" "" out "${lcv}")
    configure_file (${lcv} ${out} @ONLY)
    list (APPEND install-programs-in-done ${CMAKE_CURRENT_BINARY_DIR}/${out})
endforeach ()

#
# "make install" rules
#
###install (FILES ${install-files} DESTINATION scripts)
###install (PROGRAMS ${install-programs} DESTINATION scripts)
install (PROGRAMS ${install-programs-in-done} DESTINATION scripts)
