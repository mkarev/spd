@echo off

mkdir "build_x64"
pushd "build_x64"
cmake -G "Visual Studio 15 2017 Win64" ..
cmake --build . --config Debug
popd
