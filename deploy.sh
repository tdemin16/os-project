MAKE="make build"
CLEAN="make clean"
BUILD_DIR="bin/"
sudo rm /tmp/A_R_Comm
$CLEAN
$MAKE
cd $BUILD_DIR
./M ../src/Analyzer