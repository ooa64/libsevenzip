/*
 * libsevenzip - C++ binding to the 7zip dynamic library
 * Compress Example - Demonstrates how to create an archive
 * 
 * Copyright (C) 2025
 * Licensed under LGPL v2.1
 */

#include <libsevenzip/libsevenzip.hpp>
#include <iostream>
#include <string>
#include <vector>

// Progress callback function
bool progress_callback(uint64_t processed, uint64_t total) {
    if (total > 0) {
        double percentage = (processed * 100.0) / total;
        std::cout << "\rProgress: " << static_cast<int>(percentage) << "%" << std::flush;
    }
    return true;  // Return false to cancel operation
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <output_archive> <file1> [file2] [file3] ..." << std::endl;
        std::cerr << "Example: " << argv[0] << " output.7z file1.txt file2.txt file3.txt" << std::endl;
        return 1;
    }

    std::string output_archive = argv[1];
    std::vector<std::string> input_files;
    
    for (int i = 2; i < argc; ++i) {
        input_files.push_back(argv[i]);
    }

    try {
        // Initialize the library
        if (!sevenzip::initialize()) {
            std::cerr << "Failed to initialize libsevenzip" << std::endl;
            return 1;
        }

        std::cout << "libsevenzip version: " << sevenzip::get_version() << std::endl;
        std::cout << "Creating archive: " << output_archive << std::endl;
        std::cout << "Input files:" << std::endl;
        for (const auto& file : input_files) {
            std::cout << "  - " << file << std::endl;
        }

        // Create the archive
        std::cout << "\nCompressing..." << std::endl;
        sevenzip::Archive::create(
            output_archive,
            input_files,
            sevenzip::ArchiveFormat::SEVENZIP,
            sevenzip::CompressionMethod::LZMA2,
            sevenzip::CompressionLevel::NORMAL,
            "",  // No password
            progress_callback
        );
        std::cout << std::endl;

        std::cout << "Archive created successfully!" << std::endl;

        // Cleanup
        sevenzip::cleanup();

        return 0;

    } catch (const sevenzip::CompressionException& e) {
        std::cerr << "Error during compression: " << e.what() << std::endl;
        return 2;
    } catch (const sevenzip::Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 3;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 4;
    }
}
