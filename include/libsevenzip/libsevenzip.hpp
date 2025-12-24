/*
 * libsevenzip - C++ binding to the 7zip dynamic library
 * Copyright (C) 2025
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#ifndef LIBSEVENZIP_HPP
#define LIBSEVENZIP_HPP

/**
 * @file libsevenzip.hpp
 * @brief Main header file for libsevenzip library
 * 
 * This library provides a modern C++ interface to the 7zip dynamic library,
 * allowing easy manipulation of various archive formats including 7z, zip,
 * tar, gzip, bzip2, and more.
 * 
 * Example usage:
 * @code
 * #include <libsevenzip/libsevenzip.hpp>
 * 
 * // Extract an archive
 * sevenzip::Archive archive;
 * archive.open("example.7z");
 * archive.extract_all("/output/directory");
 * archive.close();
 * 
 * // Create an archive
 * std::vector<std::string> files = {"file1.txt", "file2.txt"};
 * sevenzip::Archive::create("output.7z", files);
 * @endcode
 */

#include "types.hpp"
#include "exceptions.hpp"
#include "archive.hpp"

namespace sevenzip {

/**
 * Get library version string
 * @return Version string in format "major.minor.patch"
 */
const char* get_version();

/**
 * Initialize the library
 * Must be called before using any library functions
 * @return true if initialization successful
 */
bool initialize();

/**
 * Cleanup library resources
 * Should be called when done using the library
 */
void cleanup();

} // namespace sevenzip

#endif // LIBSEVENZIP_HPP
