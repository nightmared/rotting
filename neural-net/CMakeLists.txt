cmake_minimum_required (VERSION 3.8.0)
project (morpion)
set(EXECUTABLE_OUTPUT_PATH bin)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

file(GLOB_RECURSE source_files src/*)
add_executable (morpion ${source_files})
target_link_libraries(morpion m ${GTK3_LIBRARIES})
