#!/bin/bash

# setup environment

# build
export SRC=deras
make -C ../build

# run
./../build/bin/debug/$SRC
