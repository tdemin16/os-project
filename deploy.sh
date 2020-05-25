#!/bin/bash
MAKE="make build"
CLEAN="make clean"
BUILD_DIR="bin/"

$CLEAN
$MAKE

if [ $? -eq 0 ]; then 
    cd $BUILD_DIR
<<<<<<< HEAD
    ./M ../src/
=======
    ./M ../src
>>>>>>> b2512110a82d58244b942ab4d8f90ba94a397450
else
    echo Fail during building.
fi
