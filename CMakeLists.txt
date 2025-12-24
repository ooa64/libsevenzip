cmake_minimum_required(VERSION 3.10)
project(libsevenzip VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library source files
set(LIBRARY_SOURCES
    src/libsevenzip.cpp
    src/exceptions.cpp
    src/archive.cpp
)

# Library header files
set(LIBRARY_HEADERS
    include/libsevenzip/libsevenzip.hpp
    include/libsevenzip/types.hpp
    include/libsevenzip/exceptions.hpp
    include/libsevenzip/archive.hpp
)

# Create the library
add_library(sevenzip SHARED ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})
add_library(sevenzip_static STATIC ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})

# Set library properties
set_target_properties(sevenzip PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${LIBRARY_HEADERS}"
)

set_target_properties(sevenzip_static PROPERTIES
    OUTPUT_NAME sevenzip
    PUBLIC_HEADER "${LIBRARY_HEADERS}"
)

# Include directories
target_include_directories(sevenzip
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(sevenzip_static
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler warnings
if(MSVC)
    target_compile_options(sevenzip PRIVATE /W4)
    target_compile_options(sevenzip_static PRIVATE /W4)
else()
    target_compile_options(sevenzip PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(sevenzip_static PRIVATE -Wall -Wextra -pedantic)
endif()

# Examples (optional)
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_executable(extract_example examples/extract_example.cpp)
    target_link_libraries(extract_example PRIVATE sevenzip)
    
    add_executable(compress_example examples/compress_example.cpp)
    target_link_libraries(compress_example PRIVATE sevenzip)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS sevenzip sevenzip_static
    EXPORT sevenzipTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libsevenzip
)

install(EXPORT sevenzipTargets
    FILE sevenzipTargets.cmake
    NAMESPACE sevenzip::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sevenzip
)

# Create and install package config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/sevenzipConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file(cmake/sevenzipConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/sevenzipConfig.cmake"
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/sevenzipConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/sevenzipConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sevenzip
)
