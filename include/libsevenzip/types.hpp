/*
 * libsevenzip - C++ binding to the 7zip dynamic library
 * Copyright (C) 2025
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#ifndef LIBSEVENZIP_TYPES_HPP
#define LIBSEVENZIP_TYPES_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace sevenzip {

/**
 * Compression method enumeration
 */
enum class CompressionMethod {
    LZMA,
    LZMA2,
    PPMD,
    BZIP2,
    DEFLATE,
    COPY
};

/**
 * Compression level enumeration
 */
enum class CompressionLevel {
    NONE = 0,
    FASTEST = 1,
    FAST = 3,
    NORMAL = 5,
    MAXIMUM = 7,
    ULTRA = 9
};

/**
 * Archive format enumeration
 */
enum class ArchiveFormat {
    SEVENZIP,
    ZIP,
    GZIP,
    BZIP2,
    TAR,
    XZ,
    AUTO  // Auto-detect format
};

/**
 * File information structure
 */
struct FileInfo {
    std::string path;
    uint64_t size;
    uint64_t packed_size;
    uint64_t crc;
    bool is_directory;
    uint64_t modified_time;
    
    FileInfo() : size(0), packed_size(0), crc(0), is_directory(false), modified_time(0) {}
};

/**
 * Progress callback function type
 * Parameters: processed_bytes, total_bytes
 * Returns: true to continue, false to cancel
 */
using ProgressCallback = bool (*)(uint64_t, uint64_t);

} // namespace sevenzip

#endif // LIBSEVENZIP_TYPES_HPP
