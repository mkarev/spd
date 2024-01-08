@echo off

set VS_WHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /F "usebackq tokens=1,* delims=." %%v in (`%VS_WHERE% -latest -property catalog.productDisplayVersion`) do set VS_GENERATOR=Visual Studio %%v
for /F "usebackq tokens=*"            %%y in (`%VS_WHERE% -latest -property catalog.productLineVersion`)    do set VS_GENERATOR=%VS_GENERATOR% %%y

mkdir "build_x64"
pushd "build_x64"

cmake -G "%VS_GENERATOR%" -A x64 .. || exit /b 1
cmake --build . --config RelWithDebInfo
ctest -C RelWithDebInfo --output-on-failure

popd
