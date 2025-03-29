cmake_minimum_required(VERSION 3.10)

# Set project version
set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 3)
set(CMAKE_PROJECT_VERSION_PATCH 2)
set(FULL_VERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH})

project(MPQCLI VERSION ${FULL_VERSION})

# Determine git commit hash
execute_process (
    COMMAND git rev-parse HEAD
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE GIT_COMMIT_HASH
)

# Set project defaults
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MPQCLI_BINARY_DIR}/bin")

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

set(STORM_USE_BUNDLED_LIBRARIES ON CACHE BOOL "Build using bundled libraries" FORCE)
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

# Add configuration file including version variable
configure_file(src/mpqcli.h.in mpqcli.h)

# Create application binary release
add_executable(mpqcli src/main.cpp src/mpq.cpp src/helpers.cpp)

# Detect if we are using MSVC
if (MSVC)
    # Determine the library name based on build type and configuration
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        if (CMAKE_CXX_FLAGS MATCHES "/D_UNICODE")
            set(STORMLIB_LIB_NAME "StormLibDUD.lib") # Debug Unicode CRT-DLL version
        else()
            set(STORMLIB_LIB_NAME "StormLibDAD.lib") # Debug Ansi CRT-DLL version
        endif()
    else() # Release build
        if (CMAKE_CXX_FLAGS MATCHES "/D_UNICODE")
            set(STORMLIB_LIB_NAME "StormLibRUD.lib") # Release Unicode CRT-DLL version
        else()
            set(STORMLIB_LIB_NAME "StormLibRAD.lib") # Release Ansi CRT-DLL version
        endif()
    endif()

    # Add the library to the target
    target_link_libraries(mpqcli PRIVATE ${STORMLIB_LIB_NAME})
else()
    # For non-MSVC compilers, link the standard StormLib library
    target_link_libraries(mpqcli PRIVATE storm)
endif()

# target_link_libraries(mpqcli storm)
target_link_libraries(mpqcli CLI11::CLI11)
target_include_directories(mpqcli PUBLIC "${PROJECT_BINARY_DIR}")
