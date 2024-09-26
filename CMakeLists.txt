cmake_minimum_required(VERSION 3.21)
project(
  Heretic
  VERSION     0.1
  DESCRIPTION "Final Fantasy Tactics Toolkit"
  LANGUAGES   CXX)

# Use C++20 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable compile commands export for tools like clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable address sanitizer, undefined behavior sanitizer, and extra debug flags in Debug mode
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode: enabling sanitizers and extra warnings")

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(
            -Wall                # Enable all standard warnings
            -Wextra              # Enable additional warnings
            -Wshadow             # Warn if variables shadow others
            -Wpedantic           # Strict standard compliance warnings
            -Wformat=2           # Warn on dangerous printf-like format strings
            -Werror              # Treat all warnings as errors
            -Wuninitialized      # Warn if uninitialized variables are used
            -fsanitize=address   # Enable AddressSanitizer
            -fsanitize=undefined # Enable UndefinedBehaviorSanitizer
            -g                   # Generate debug information
        )

        # Link sanitizers
        add_link_options(
            -fsanitize=address
            -fsanitize=undefined
        )
    endif()

else()
    message(STATUS "Release mode: enabling optimizations")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3") # Enable optimizations in Release mode
endif()

# ImGui
file(GLOB IMGUI_SOURCES "lib/imgui/*.cpp" "lib/imgui/backends/imgui_impl_glfw.cpp" "lib/imgui/backends/imgui_impl_opengl3.cpp")
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PRIVATE lib/imgui)

# Heretic executable
file(GLOB_RECURSE HERETIC_SOURCES "src/*.cpp")
add_executable(heretic ${HERETIC_SOURCES})
target_include_directories(heretic SYSTEM PRIVATE lib/sokol lib/sokol/util lib/imgui lib/stb lib/glm)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(heretic imgui GL X11 Xi Xcursor m)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND IMGUI_SOURCES "lib/imgui/backends/imgui_impl_osx.mm")
    target_include_directories(imgui PRIVATE /opt/homebrew/include)
    target_link_libraries(heretic imgui "-framework Foundation" "-framework OpenGL" "-framework Cocoa")
    set_source_files_properties(src/main.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
