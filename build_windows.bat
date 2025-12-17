@echo off
setlocal

rem This script is designed to build the ArcaneLock project on Windows.
rem It assumes you have CMake, a C++ compiler (MSVC or MinGW-w64),
rem Qt6, and libsodium installed and configured.

rem IMPORTANT: You MUST edit the paths and generator below to match your setup.

rem --- Configure Qt 6 Installation Directory ---
rem Set this to the root of your Qt 6 installation (e.g., C:\Qt\6.x.x\msvc2019_64 or C:\Qt\6.x.x\mingw_64)
set "QT_INSTALL_DIR=C:\path\to\your\Qt\6.x.x"

rem --- Configure libsodium Installation Directory ---
rem Set this to the root where you extracted libsodium (e.g., C:\libsodium)
set "LIBSODIUM_INSTALL_DIR=C:\path\to\your\libsodium"

rem --- Choose CMake Generator ---
rem Uncomment ONE of the following lines to select your compiler.
rem For Visual Studio (recommended for MSVC):
set "CMAKE_GENERATOR=Visual Studio 17 2022"
rem For MinGW-w64 (recommended for MinGW):
rem set "CMAKE_GENERATOR=MinGW Makefiles"

rem --- Environment Setup for Qt ---
rem This helps CMake find Qt. Adjust the path structure if your Qt installation differs.
if exist "%QT_INSTALL_DIR%\bin\qmake.exe" (
    set "PATH=%QT_INSTALL_DIR%\bin;%PATH%"
    set "CMAKE_PREFIX_PATH=%QT_INSTALL_DIR%"
) else (
    echo WARNING: Qt installation path not found or invalid: %QT_INSTALL_DIR%
    echo Please set QT_INSTALL_DIR correctly in this script.
    goto :eof
)

rem --- Environment Setup for libsodium (Optional but recommended) ---
rem This helps CMake find libsodium.
if exist "%LIBSODIUM_INSTALL_DIR%\include\sodium.h" (
    set "CMAKE_PREFIX_PATH=%LIBSODIUM_INSTALL_DIR%;%CMAKE_PREFIX_PATH%"
) else (
    echo WARNING: libsodium installation path not found or invalid: %LIBSODIUM_INSTALL_DIR%
    echo Please set LIBSODIUM_INSTALL_DIR correctly in this script.
    rem We will proceed without libsodium for now, but build might fail.
)


set "BUILD_DIR=build_windows"

rem Create build directory if it doesn't exist
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

rem Navigate into the build directory
pushd "%BUILD_DIR%"

echo Configuring CMake project for Windows...
cmake -G "%CMAKE_GENERATOR%" -DCMAKE_PREFIX_PATH="%CMAKE_PREFIX_PATH%" ..

if %errorlevel% neq 0 (
    echo CMake configuration failed.
    goto :error
)

echo Building project...
rem For Visual Studio generators, --config Release is important.
rem For MinGW Makefiles, it might be implicit or not needed.
if "%CMAKE_GENERATOR%"=="MinGW Makefiles" (
    cmake --build . 
) else (
    cmake --build . --config Release
)

if %errorlevel% neq 0 (
    echo Build failed.
    goto :error
)

rem Navigate back to the original directory
popd

echo Build complete.
echo The executable should be in "%BUILD_DIR%\Release\arcanelock.exe" (or "%BUILD_DIR%\arcanelock.exe" for MinGW builds).
goto :eof

:error
popd
echo An error occurred during the build process.
exit /b 1
