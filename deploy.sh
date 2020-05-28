#!/bin/bash
MAKE="make build"
CLEAN="make clean"
BUILD_DIR="bin/"

$CLEAN
$MAKE

if [ $? -eq 0 ]; then 
    cd $BUILD_DIR
<<<<<<< HEAD
    ./M
=======
    ./M ../test/test1.txt
>>>>>>> 6f2e017f6ed55fa1fb0fd4328969aa2dadd78bb3
else
    echo Fail during building.
fi
