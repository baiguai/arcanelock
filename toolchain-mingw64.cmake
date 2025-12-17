# toolchain-mingw64.cmake
# CMake toolchain file for cross-compiling to Windows (MinGW-w64) from Linux.

# Specify the target system
SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the compilers
SET(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# Where to find the target environment.
# This should point to the root of your MinGW-w64 installation,
# or where your cross-compiled libraries (like Qt and libsodium) are located.
# For standard apt installations on Debian/Ubuntu, it often looks like:
# SET(CMAKE_FIND_ROOT_PATH "/usr/x86_64-w64-mingw32")
# or if you have custom built/installed cross-libs:
# SET(CMAKE_FIND_ROOT_PATH "/path/to/your/mingw64/root;/path/to/cross-compiled/qt;/path/to/cross-compiled/libsodium")
#
# For now, we will assume standard apt installation location if CMAKE_FIND_ROOT_PATH is not already set.
# The user might need to adjust this.
IF(NOT CMAKE_FIND_ROOT_PATH)
    SET(CMAKE_FIND_ROOT_PATH "/usr/x86_64-w64-mingw32")
ENDIF()


# Adjust how CMake searches for programs, libraries and headers
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER) # Do not search for programs in the target environment
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)  # Only search for libraries in the target environment
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)  # Only search for headers in the target environment

# You might need to add specific paths for Qt6 if it's not found automatically.
# Example:
LIST(APPEND CMAKE_PREFIX_PATH "/mingw_64") # User-provided path for cross-compiled Qt6

# Similarly for libsodium if it's not found:
# LIST(APPEND CMAKE_PREFIX_PATH "/path/to/your/cross-compiled/libsodium")
