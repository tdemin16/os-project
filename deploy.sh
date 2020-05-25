#!/bin/bash
MAKE="make build"
CLEAN="make clean"
BUILD_DIR="bin/"

$CLEAN
$MAKE

if [ $? -eq 0 ]; then 
    cd $BUILD_DIR
    ./M ../src/
else
    echo Fail during building.
fi
