cmake_minimum_required(VERSION 3.16)
project(ArcaneLock CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Include FetchContent ---
include(FetchContent)

# --- Fetch Glad (OpenGL loader) ---
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.36
)
FetchContent_MakeAvailable(glad)

# --- Fetch GLFW (Windowing library) ---
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# --- Fetch Dear ImGui ---
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.89.9
)
FetchContent_MakeAvailable(imgui)

find_package(OpenGL REQUIRED)

# --- Add ImGui sources to our executable ---
file(GLOB IMGUI_SOURCES
  "${CMAKE_BINARY_DIR}/_deps/imgui-src/imgui.cpp"
  "${CMAKE_BINARY_DIR}/_deps/imgui-src/imgui_draw.cpp"
  "${CMAKE_BINARY_DIR}/_deps/imgui-src/imgui_tables.cpp"
  "${CMAKE_BINARY_DIR}/_deps/imgui-src/imgui_widgets.cpp"
  "${CMAKE_BINARY_DIR}/_deps/imgui-src/backends/imgui_impl_glfw.cpp"
  "${CMAKE_BINARY_DIR}/_deps/imgui-src/backends/imgui_impl_opengl3.cpp"
)

# --- Add our executable ---
add_executable(arcanelock src/main.cpp ${IMGUI_SOURCES} src/model/Node.cpp)

# --- Add include directories ---
target_include_directories(arcanelock
  PRIVATE
  ${CMAKE_BINARY_DIR}/_deps/imgui-src
  ${CMAKE_BINARY_DIR}/_deps/imgui-src/backends
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# --- Link libraries ---
target_link_libraries(arcanelock
  PRIVATE
  glad
  glfw
)

# --- Platform-specific libraries ---
if (WIN32)
    target_link_libraries(arcanelock PRIVATE gdi32)
endif()

if (APPLE)
    target_link_libraries(arcanelock PRIVATE "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
endif()

find_package(Threads REQUIRED)
if (UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(X11 REQUIRED x11)
    target_link_libraries(arcanelock PRIVATE Threads::Threads ${X11_LIBRARIES} OpenGL::GL)
endif()
