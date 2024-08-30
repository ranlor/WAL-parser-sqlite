#! /bin/bash
rm -rf build
mkdir build
pushd build

cmake .. -DCMAKE_VERBOSE_MAKEFILE=OFF -DCMAKE_BUILD_TYPE=${1}
cmake --build . -j$(nproc)

cp wal-parser ..

popd
