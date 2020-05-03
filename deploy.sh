CLEAN="make clean"
MAKE="make build"
BUILD_DIR="bin/"

$CLEAN
$MAKE
cd $BUILD_DIR
./A ../src/Analyzer