@echo off

mkdir "build_x64"
pushd "build_x64"
cmake -G  "Visual Studio 16 2019" -A x64 .. || (
    del CMakeCache.txt
    cmake -G "Visual Studio 15 2017 Win64" .. || exit /b 1
)
cmake --build . --config RelWithDebInfo
ctest -C RelWithDebInfo --output-on-failure
popd
