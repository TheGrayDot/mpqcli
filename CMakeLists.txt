project(MPQCLI)
cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MPQCLI_BINARY_DIR}/bin")

if (NOT EXISTS "${MPQCLI_SOURCE_DIR}/StormLib/CMakeLists.txt")
    message(FATAL_ERROR
"Missing dependency: StormLib
mpqcli requires the StormLib library.
It is provided as a submodule of this repository.
Did you forget to execute the following commands?
   git submodule init
   git submodule update")
endif()

set(STORM_USE_BUNDLED_LIBRARIES ON CACHE BOOL "Build using bundles libraries")
add_subdirectory(StormLib)

include_directories(
    "${MPQCLI_SOURCE_DIR}/StormLib/src/"
    "${MPQCLI_SOURCE_DIR}/include/"
)

add_executable(mpqcli src/mpqcli.cpp src/info.cpp src/verify.cpp src/mpq.cpp src/patch.cpp src/helpers.cpp)

target_link_libraries(mpqcli storm)

target_include_directories(mpqcli PRIVATE ${MPQCLI_SOURCE_DIR}/include )
