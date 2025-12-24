# libsevenzip

C++ binding to the 7zip dynamic library

## Overview

`libsevenzip` is a modern C++ library that provides a clean and easy-to-use interface to the 7-Zip dynamic library. It allows you to compress and decompress various archive formats including 7z, ZIP, TAR, GZIP, BZIP2, and more.

## Features

- **Modern C++ API**: Clean, RAII-based interface using C++14
- **Multiple Archive Formats**: Support for 7z, ZIP, TAR, GZIP, BZIP2, XZ, and more
- **Compression & Extraction**: Full support for creating and extracting archives
- **Progress Callbacks**: Monitor compression and extraction progress
- **Password Protection**: Support for encrypted archives
- **Exception-based Error Handling**: Clear error messages using exceptions
- **Cross-platform**: Works on Linux, macOS, and Windows

## Building

### Using CMake

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

### Using Make

```bash
make
sudo make install
```

### Build Options

- `BUILD_EXAMPLES`: Build example programs (default: ON)

```bash
cmake -DBUILD_EXAMPLES=OFF ..
```

## Usage

### Basic Example - Extracting an Archive

```cpp
#include <libsevenzip/libsevenzip.hpp>

int main() {
    // Initialize the library
    sevenzip::initialize();
    
    // Open and extract an archive
    sevenzip::Archive archive;
    archive.open("example.7z");
    archive.extract_all("/output/directory");
    archive.close();
    
    // Cleanup
    sevenzip::cleanup();
    return 0;
}
```

### Basic Example - Creating an Archive

```cpp
#include <libsevenzip/libsevenzip.hpp>

int main() {
    // Initialize the library
    sevenzip::initialize();
    
    // Create an archive
    std::vector<std::string> files = {"file1.txt", "file2.txt", "file3.txt"};
    sevenzip::Archive::create("output.7z", files);
    
    // Cleanup
    sevenzip::cleanup();
    return 0;
}
```

### Advanced Example - With Progress Callback

```cpp
#include <libsevenzip/libsevenzip.hpp>
#include <iostream>

bool progress_callback(uint64_t processed, uint64_t total) {
    double percentage = (processed * 100.0) / total;
    std::cout << "Progress: " << percentage << "%\r" << std::flush;
    return true;  // Return false to cancel
}

int main() {
    sevenzip::initialize();
    
    try {
        sevenzip::Archive archive;
        archive.open("large_archive.7z");
        archive.extract_all("/output", progress_callback);
    } catch (const sevenzip::Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    sevenzip::cleanup();
    return 0;
}
```

### Creating Password-Protected Archives

```cpp
std::vector<std::string> files = {"secret.txt"};
sevenzip::Archive::create(
    "encrypted.7z",
    files,
    sevenzip::ArchiveFormat::SEVENZIP,
    sevenzip::CompressionMethod::LZMA2,
    sevenzip::CompressionLevel::MAXIMUM,
    "my_password"  // Password
);
```

## API Documentation

### Main Classes

#### `sevenzip::Archive`

The main class for archive operations.

**Methods:**
- `void open(const std::string& path, ArchiveFormat format = AUTO, const std::string& password = "")` - Open an archive
- `void close()` - Close the current archive
- `bool is_open() const` - Check if archive is open
- `size_t get_item_count() const` - Get number of items in archive
- `FileInfo get_item_info(size_t index) const` - Get info about specific item
- `std::vector<FileInfo> get_all_items() const` - Get info about all items
- `void extract_item(size_t index, const std::string& output_path, ProgressCallback callback = nullptr)` - Extract single item
- `void extract_all(const std::string& output_dir, ProgressCallback callback = nullptr)` - Extract all items
- `static void create(...)` - Create new archive (static method)

### Enumerations

#### `ArchiveFormat`
- `SEVENZIP` - 7z format
- `ZIP` - ZIP format
- `GZIP` - GZIP format
- `BZIP2` - BZIP2 format
- `TAR` - TAR format
- `XZ` - XZ format
- `AUTO` - Auto-detect format

#### `CompressionMethod`
- `LZMA` - LZMA compression
- `LZMA2` - LZMA2 compression (recommended)
- `PPMD` - PPMd compression
- `BZIP2` - BZip2 compression
- `DEFLATE` - Deflate compression
- `COPY` - No compression

#### `CompressionLevel`
- `NONE` - No compression (0)
- `FASTEST` - Fastest compression (1)
- `FAST` - Fast compression (3)
- `NORMAL` - Normal compression (5)
- `MAXIMUM` - Maximum compression (7)
- `ULTRA` - Ultra compression (9)

### Exception Classes

All exceptions inherit from `sevenzip::Exception`:

- `ArchiveOpenException` - Cannot open archive
- `FormatException` - Format not supported or cannot be detected
- `ExtractionException` - Error during extraction
- `CompressionException` - Error during compression
- `PasswordException` - Password required or incorrect
- `CancelledException` - Operation cancelled by user

## Examples

Example programs are provided in the `examples/` directory:

- `extract_example.cpp` - Demonstrates archive extraction
- `compress_example.cpp` - Demonstrates archive creation

To build and run examples:

```bash
cd build
./extract_example archive.7z /tmp/output
./compress_example output.7z file1.txt file2.txt
```

## Requirements

- C++14 compatible compiler
- CMake 3.10 or higher (for CMake build)
- 7-Zip dynamic library (7z.dll on Windows, 7z.so on Linux, 7z.dylib on macOS)

## License

This library is licensed under the GNU Lesser General Public License v2.1 (LGPL-2.1).
See the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

## Author

libsevenzip project

## Acknowledgments

This library builds upon the excellent 7-Zip SDK by Igor Pavlov.
