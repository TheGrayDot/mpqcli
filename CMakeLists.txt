cmake_minimum_required(VERSION 3.10)

# Set project version
project(MPQCLI VERSION 0.6.2)

# Determine git commit hash
execute_process (
    COMMAND git rev-parse HEAD
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE GIT_COMMIT_HASH
)

# Set project defaults
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MPQCLI_BINARY_DIR}/bin")

if (MSVC)
    add_compile_definitions(__STORMLIB_NO_STATIC_LINK__)
    set(CMAKE_GENERATOR_PLATFORM x64)
endif()

# Handle StormLib dependency
if (NOT EXISTS "${MPQCLI_SOURCE_DIR}/extern/StormLib/CMakeLists.txt")
    message(FATAL_ERROR
"Missing dependency: StormLib
mpqcli requires the StormLib library.
It is provided as a submodule of this repository.
Did you forget to execute the following commands?
   git submodule init
   git submodule update")
endif()

add_subdirectory(extern/StormLib)

# Handle CLI11 dependency
if (NOT EXISTS "${MPQCLI_SOURCE_DIR}/extern/CLI11/CMakeLists.txt")
    message(FATAL_ERROR
"Missing dependency: CLI11
mpqcli requires the CLI11 library.
It is provided as a submodule of this repository.
Did you forget to execute the following commands?
   git submodule init
   git submodule update")
endif()

add_subdirectory(extern/CLI11)

configure_file(src/mpqcli.h.in mpqcli.h)

add_executable(mpqcli src/main.cpp src/mpq.cpp src/helpers.cpp)

add_dependencies(mpqcli storm)

target_include_directories(mpqcli PRIVATE /extern/StormLib/src)

target_link_libraries(mpqcli PRIVATE storm)

target_link_libraries(mpqcli PRIVATE CLI11::CLI11)

target_include_directories(mpqcli PUBLIC "${PROJECT_BINARY_DIR}")
