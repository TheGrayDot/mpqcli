cmake_minimum_required(VERSION 3.2)

project(MPQCLI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MPQCLI_BINARY_DIR}/bin")

if (NOT EXISTS "${MPQCLI_SOURCE_DIR}/extern/StormLib/CMakeLists.txt")
    message(FATAL_ERROR
"Missing dependency: StormLib
mpqcli requires the StormLib library.
It is provided as a submodule of this repository.
Did you forget to execute the following commands?
   git submodule init
   git submodule update")
endif()

if (NOT EXISTS "${MPQCLI_SOURCE_DIR}/extern/CLI11/CMakeLists.txt")
    message(FATAL_ERROR
"Missing dependency: CLI11
mpqcli requires the CLI11 library.
It is provided as a submodule of this repository.
Did you forget to execute the following commands?
   git submodule init
   git submodule update")
endif()

set(STORM_USE_BUNDLED_LIBRARIES ON CACHE BOOL "Build using bundled libraries")
add_subdirectory(extern/StormLib)
add_subdirectory(extern/CLI11)

add_executable(mpqcli src/main.cpp src/mpq.cpp src/helpers.cpp)

target_link_libraries(mpqcli storm)
target_link_libraries(mpqcli CLI11::CLI11)
