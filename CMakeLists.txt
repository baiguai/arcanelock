cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(ArcaneLock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt 6
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Add application executable
# Ensure all source files are listed here
add_executable(arcanelock src/main.cpp src/MainWindow.cpp)

# Add the 'src' directory to the include paths so header files are found
target_include_directories(arcanelock PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link Qt libraries and enable automatic MOC processing
target_link_libraries(arcanelock PRIVATE Qt6::Widgets)

# For projects using Qt, it's good practice to ensure that the necessary
# Qt modules are available for all configurations.
# This also handles moc'ing and ui processing automatically if source files
# are correctly added to the target.
set_property(TARGET arcanelock PROPERTY AUTOMOC ON)
set_property(TARGET arcanelock PROPERTY AUTORCC ON) # If we add resource files later
set_property(TARGET arcanelock PROPERTY AUTOUIC ON) # If we add .ui files later