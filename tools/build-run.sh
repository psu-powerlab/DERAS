#!/bin/bash

# setup environment
export CPU=x86_64
export OS=linux
export VARIANT=debug
export AJ_ROOT=$HOME/src/core-alljoyn/build/$OS/$CPU/$VARIANT/dist/cpp
export AJ_LIB=$AJ_ROOT/lib
export AJ_INC=$AJ_ROOT/inc
export LD_LIBRARY_PATH=$AJ_LIB:$LD_LIBRARY_PATH
export BOOST_INC=$HOME/src/boost_1_66_0

# alljoyn router
# first kill any daemon's that are currently running, then start new daemon
# the last /dev/null 2>&1 & has all stdout set to null and runs in background
export AJ_ROUTER= #stand-alone
killall alljoyn-daemon
sleep 1
$AJ_ROOT/bin/alljoyn-daemon --config-file=../data/rn-config.xml > /dev/null 2>&1 &
sleep 5

# build
export SRC=deras
make -C ../build

# run
./../build/bin/debug/$SRC -c ./../data/config.ini
