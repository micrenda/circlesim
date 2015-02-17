#!/bin/sh
# This file set the CC and CXX enviroment variables. Use it with source command
export CC="/home/mrenda/bin/gcc-4.9.2/usr/local/bin/gcc"
export CXX="/home/mrenda/bin/gcc-4.9.2/usr/local/bin/g++"

DATA=/data/mrenda

export C_INCLUDE_PATH=""
export CPLUS_INCLUDE_PATH="$DATA/lib/libconfig-1.4.9/include:$DATA/lib/boost_1_55_0/include:$DATA/lib/armadillo-4.600.4/usr/local/include:/data/mrenda/lib/lua-5.2.3/include"

# ld does not read this variable
export LD_LIBRARY_PATH="$DATA/lib/boost_1_55_0/lib:$DATA/lib/libconfig-1.4.9/lib:$DATA/lib/armadillo-4.600.4/usr/local/lib64:$DATA/lib/lua-5.2.3:$DATA/bin/gcc-4.9.2/usr/local/lib64"
# this is the variable LD read
export LDFLAGS="-L$DATA/lib/boost_1_55_0/lib -L$DATA/lib/libconfig-1.4.9/lib -L$DATA/lib/armadillo-4.600.4/usr/local/lib64 -L$DATA/lib/lua-5.2.3 -L$DATA/bin/gcc-4.9.2/usr/local/lib64/"
