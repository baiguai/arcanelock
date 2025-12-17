cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(ArcaneLock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt 6
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Manually find libsodium
find_library(SODIUM_LIBRARY NAMES sodium)
find_path(SODIUM_INCLUDE_DIR NAMES sodium.h)

if (NOT SODIUM_LIBRARY OR NOT SODIUM_INCLUDE_DIR)
    message(FATAL_ERROR "libsodium not found. Please install libsodium-dev (Debian/Ubuntu) or libsodium (macOS/other Linux).")
endif()

# Add application executable
# Ensure all source files are listed here
add_executable(arcanelock src/main.cpp src/MainWindow.cpp src/OpenDbDialog.cpp src/SetMasterPasswordDialog.cpp)

# Add the 'src' directory to the include paths so header files are found
target_include_directories(arcanelock PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${SODIUM_INCLUDE_DIR})

# Link Qt libraries and enable automatic MOC processing
target_link_libraries(arcanelock PRIVATE Qt6::Widgets ${SODIUM_LIBRARY})

# For projects using Qt, it's good practice to ensure that the necessary
# Qt modules are available for all configurations.
# This also handles moc'ing and ui processing automatically if source files
# are correctly added to the target.
set_property(TARGET arcanelock PROPERTY AUTOMOC ON)
set_property(TARGET arcanelock PROPERTY AUTORCC ON) # If we add resource files later
set_property(TARGET arcanelock PROPERTY AUTOUIC ON) # If we add .ui files later