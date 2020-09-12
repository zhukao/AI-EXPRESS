rm -rf build.properties.local build output
cp build.properties.local.aarch64 build.properties.local
mkdir build
cd build
cmake ..
make
make install
make upload