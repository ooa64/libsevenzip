/*
 * libsevenzip - C++ binding to the 7zip dynamic library
 * Copyright (C) 2025
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#ifndef LIBSEVENZIP_ARCHIVE_HPP
#define LIBSEVENZIP_ARCHIVE_HPP

#include "types.hpp"
#include <memory>
#include <string>
#include <vector>

namespace sevenzip {

/**
 * Archive class for reading and writing archives
 */
class Archive {
public:
    /**
     * Constructor
     */
    Archive();
    
    /**
     * Destructor
     */
    ~Archive();
    
    // Disable copy
    Archive(const Archive&) = delete;
    Archive& operator=(const Archive&) = delete;
    
    // Enable move
    Archive(Archive&&) noexcept;
    Archive& operator=(Archive&&) noexcept;
    
    /**
     * Open an existing archive for reading
     * @param path Path to the archive file
     * @param format Archive format (AUTO for auto-detection)
     * @param password Optional password for encrypted archives
     */
    void open(const std::string& path, 
              ArchiveFormat format = ArchiveFormat::AUTO,
              const std::string& password = "");
    
    /**
     * Check if archive is currently open
     * @return true if archive is open
     */
    bool is_open() const;
    
    /**
     * Close the currently open archive
     */
    void close();
    
    /**
     * Get the number of items in the archive
     * @return Number of items
     */
    size_t get_item_count() const;
    
    /**
     * Get information about a specific item in the archive
     * @param index Item index (0-based)
     * @return FileInfo structure with item information
     */
    FileInfo get_item_info(size_t index) const;
    
    /**
     * Get information about all items in the archive
     * @return Vector of FileInfo structures
     */
    std::vector<FileInfo> get_all_items() const;
    
    /**
     * Extract a specific item from the archive
     * @param index Item index (0-based)
     * @param output_path Destination path for extracted file
     * @param callback Optional progress callback
     */
    void extract_item(size_t index, 
                     const std::string& output_path,
                     ProgressCallback callback = nullptr);
    
    /**
     * Extract all items from the archive
     * @param output_dir Destination directory for extracted files
     * @param callback Optional progress callback
     */
    void extract_all(const std::string& output_dir,
                    ProgressCallback callback = nullptr);
    
    /**
     * Create a new archive and add files to it
     * @param archive_path Path for the new archive file
     * @param files List of file paths to add to the archive
     * @param format Archive format
     * @param method Compression method
     * @param level Compression level
     * @param password Optional password for encryption
     * @param callback Optional progress callback
     */
    static void create(const std::string& archive_path,
                      const std::vector<std::string>& files,
                      ArchiveFormat format = ArchiveFormat::SEVENZIP,
                      CompressionMethod method = CompressionMethod::LZMA2,
                      CompressionLevel level = CompressionLevel::NORMAL,
                      const std::string& password = "",
                      ProgressCallback callback = nullptr);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace sevenzip

#endif // LIBSEVENZIP_ARCHIVE_HPP
