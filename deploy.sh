MAKE="make build"
CLEAN="make clean"
BUILD_DIR="bin/"

$CLEAN
$MAKE
cd $BUILD_DIR
./A ../src/Analyzer