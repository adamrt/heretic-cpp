cmake_minimum_required(VERSION 3.21)
project(
  Heretic
  VERSION     0.1
  DESCRIPTION "Final Fantasy Tactics Toolkit"
  LANGUAGES   CXX)

# Using C++11 standard to allow building on PSVita.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Imgui
file(GLOB IMGUI_SOURCES "lib/imgui/*.cpp" "lib/imgui/backends/imgui_impl_glfw.cpp" "lib/imgui/backends/imgui_impl_opengl3.cpp")
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PRIVATE lib/imgui)

# Heretic
file(GLOB_RECURSE HERETIC_SOURCES "src/*.cpp")
add_executable(heretic ${HERETIC_SOURCES})
target_include_directories(heretic SYSTEM PRIVATE lib/sokol lib/sokol/util lib/imgui lib/stb lib/glm)
target_link_libraries(heretic imgui GL X11 Xi Xcursor)
