**Download, build, and install bosen, its friends and their minimum dependencies
in a single step.**

[![Build Status](https://travis-ci.org/pdlfs/bosen-umbrella.svg?branch=master)](https://travis-ci.org/pdlfs/bosen-umbrella)

# bosen-umbrella

This package is designed to help our collaborators quickly setup bosen on
various computing platforms ranging from commodity NFS PRObE clusters to
highly-specialized Cray systems customized by different national labs. The
package a highly-automated process that downloads, builds, and installs
bosen (including many of its friends and all their dependencies) along with
demo application (matrix factorization).

Written on top of cmake, bosen-umbrella is expected to work with most major
computing platforms.

### Modules

* bosen dependencies
  * *TBD*
* bosen
  * *TBD*

### Installation

A recent CXX compiler with standard building tools including make, cmake
(used by bosen), as well as a few other common library packages.

To build bosen and install it under a specific prefix (e.g.
$HOME/bosen-umbrella/install):

```
export GIT_SSL_NO_VERIFY=true  # Assuming bash

cd $HOME
git clone https://github.com/pdlfs/bosen-umbrella.git
cd bosen-umbrella
mkdir build install

#
# $HOME/bosen-umbrella
# |-- build
# |-- cache.0
# |-- cache
# |-- install
# |-- scripts
#
cd build
cmake -DCMAKE_INSTALL_PREFIX=$HOME/bosen-umbrella/install ..
make
```

### Execution

TBD
