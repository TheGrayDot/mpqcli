# Configure version header
configure_file(mpqcli.h.in "${CMAKE_BINARY_DIR}/mpqcli.h")

# Set output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Windows specific settings
if (MSVC)
    add_compile_definitions(__STORMLIB_NO_STATIC_LINK__)
    set(CMAKE_GENERATOR_PLATFORM x64)
endif()

# Create the main executable
add_executable(mpqcli
    main.cpp
    mpq.cpp
    helpers.cpp
    locales.cpp
    gamerules.cpp
)

# Add dependencies
add_dependencies(mpqcli storm)

# Include directories
target_include_directories(mpqcli PRIVATE 
    "${CMAKE_SOURCE_DIR}/extern/StormLib/src"
    "${CMAKE_BINARY_DIR}"
)

# Link libraries
target_link_libraries(mpqcli PRIVATE storm CLI11::CLI11)
