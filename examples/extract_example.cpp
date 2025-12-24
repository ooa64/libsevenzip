/*
 * libsevenzip - C++ binding to the 7zip dynamic library
 * Extract Example - Demonstrates how to extract files from an archive
 * 
 * Copyright (C) 2025
 * Licensed under LGPL v2.1
 */

#include <libsevenzip/libsevenzip.hpp>
#include <iostream>
#include <string>

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
        std::cerr << "Usage: " << argv[0] << " <archive_file> <output_directory>" << std::endl;
        std::cerr << "Example: " << argv[0] << " archive.7z /tmp/extracted" << std::endl;
        return 1;
    }

    std::string archive_file = argv[1];
    std::string output_dir = argv[2];

    try {
        // Initialize the library
        if (!sevenzip::initialize()) {
            std::cerr << "Failed to initialize libsevenzip" << std::endl;
            return 1;
        }

        std::cout << "libsevenzip version: " << sevenzip::get_version() << std::endl;
        std::cout << "Opening archive: " << archive_file << std::endl;

        // Create archive object and open the file
        sevenzip::Archive archive;
        archive.open(archive_file);

        // Get information about archive contents
        size_t item_count = archive.get_item_count();
        std::cout << "Archive contains " << item_count << " items" << std::endl;

        // List all items
        std::cout << "\nArchive contents:" << std::endl;
        std::vector<sevenzip::FileInfo> items = archive.get_all_items();
        for (size_t i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            std::cout << "  [" << i << "] " << item.path;
            if (item.is_directory) {
                std::cout << " (directory)";
            } else {
                std::cout << " (" << item.size << " bytes)";
            }
            std::cout << std::endl;
        }

        // Extract all files
        std::cout << "\nExtracting to: " << output_dir << std::endl;
        archive.extract_all(output_dir, progress_callback);
        std::cout << std::endl;

        // Close the archive
        archive.close();
        
        std::cout << "Extraction completed successfully!" << std::endl;

        // Cleanup
        sevenzip::cleanup();

        return 0;

    } catch (const sevenzip::ArchiveOpenException& e) {
        std::cerr << "Error opening archive: " << e.what() << std::endl;
        return 2;
    } catch (const sevenzip::ExtractionException& e) {
        std::cerr << "Error during extraction: " << e.what() << std::endl;
        return 3;
    } catch (const sevenzip::Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 4;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 5;
    }
}
