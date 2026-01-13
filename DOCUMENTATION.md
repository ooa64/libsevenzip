# libsevenzip Documentation

## Overview

**libsevenzip** is a C++ wrapper library for the 7-Zip archive format library. It provides a simplified interface for working with various archive formats supported by 7-Zip, including reading, extracting, creating, and updating archives.

**Version:** 1.0

---

## Table of Contents

1. [Installation and Setup](#installation-and-setup)
2. [Core Concepts](#core-concepts)
3. [API Reference](#api-reference)
   - [Stream Interfaces](#stream-interfaces)
   - [Lib Class](#lib-class)
   - [Iarchive Class](#iarchive-class)
   - [Oarchive Class](#oarchive-class)
   - [Utility Functions](#utility-functions)
4. [Usage Examples](#usage-examples)
5. [Error Handling](#error-handling)
6. [Platform Support](#platform-support)

---

## Installation and Setup

### Installation

- Build from source using [instructions](#BUILD.md)
- or download the binary release from [repository](https://github.com/ooa64/libsevenzip)

### Requirements

- C++ compiler with C++11 support or later
- 7-Zip DLL (`7z.dll` on Windows, `7z.so` on Linux/macOS)

### Include Header

```cpp
#include "sevenzip.h"
```

### Namespace

All library classes and functions are in the `sevenzip` namespace.

---

## Core Concepts

### Stream-Based I/O

The library uses custom stream interfaces (`Istream` and `Ostream`) that you must implement to handle file I/O operations. This design allows for flexible I/O handling including:

- File system access
- Memory streams
- Network streams
- Custom stream implementations

### Archive Formats

The library supports multiple archive formats through the underlying 7-Zip library:
- 7z
- ZIP
- TAR
- GZIP
- BZIP2
- RAR (read-only)
- And many others

---

## API Reference

### Stream Interfaces

#### `Istream` - Input Stream Interface

Base interface for reading data. You must implement this interface to provide input data to the library.
  - Used to read input archive in the Iarchive class.
  - Used to read input files in the Oarchive class.
  - Passed in unopened state to Iarchive and Oarchive methods
  - In some special cases, Istream can be passed to Iarchive/Oarchive methods in an opened state.

**Required Methods:**

```cpp
virtual HRESULT Read(void* data, UInt32 size, UInt32& processed) = 0;
```
- **Purpose:** Read data from the stream
- **Parameters:**
  - `data`: Buffer to store read data
  - `size`: Number of bytes to read
  - `processed`: (Output) Number of bytes actually read
- **Returns:** `S_OK` on success, error code otherwise

**Optional Methods:**

```cpp
virtual HRESULT Open(const wchar_t* filename);
```
- **Purpose:** Open a file for reading
- **Required for:** 1) Iarchive opening, 2) input file opening for the Oarchive update method
- **Default:** Can be used for for pre-opened streams
- **Returns:** `S_OK` on success, error code otherwise

```cpp
virtual void Close();
```
- **Purpose:** Close the stream opended by Open method

```cpp
virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position);
```
- **Purpose:** Seek to a position in the stream
- **Required for:** Archive opening
- **Default:** Can be used when passing to Oarchive update method
- **Returns:** `S_OK` on success, error code otherwise

```cpp
virtual UInt64 GetSize(const wchar_t* filename);
```
- **Purpose:** Get file size for archiving
- **Required for:** Archive creation (Oarchive update method)
- **Default:** Returns 1, but it's recommended to reimplement for proper operation
- **Returns:** File size in bytes

```cpp
virtual Istream* Clone() const;
```
- **Purpose:** Create a clone of the stream
- **Required for:** Opening multi-volume archives
- **Default:** Returns nullptr, implement when multi-volume support is needed
- **Returns:** Pointer to new cloned Istream instance

```cpp
virtual bool IsDir(const wchar_t* filename) const;
```
- **Purpose:** Check if path is a directory
- **Required for:** Archive creation
- **Default:** Returns false
- **Returns:** true if path is a directory, false otherwise

```cpp
virtual UInt32 GetMode(const wchar_t* filename) const;
```
- **Purpose:** Get file mode/permissions
- **Required for:** Archive creation
- **Default:** Returns 0
- **Returns:** File mode bits (Unix-style permissions)

```cpp
virtual UInt32 GetTime(const wchar_t* filename) const;
```
- **Purpose:** Get file timestamp
- **Required for:** Archive creation
- **Default:** Returns 0
- **Returns:** Unix timestamp (seconds since epoch)

---

#### `Ostream` - Output Stream Interface

Base interface for writing data. You must implement this interface to handle output operations.
  - Used to write extracted files in the Iarchive class
  - Used to write output archive in the Oarchive class
  - Passed in unopened state to Iarchive and Oarchive methods
  - In some special cases, Ostream can be passed to Iarchive/Oarchive methods in an opened state.

**Required Methods:**

```cpp
virtual HRESULT Write(const void* data, UInt32 size, UInt32& processed) = 0;
```
- **Purpose:** Write data to the stream
- **Parameters:**
  - `data`: Buffer containing data to write
  - `size`: Number of bytes to write
  - `processed`: (Output) Number of bytes actually written
- **Returns:** `S_OK` on success, error code otherwise

**Optional Methods:**

```cpp
virtual HRESULT Open(const wchar_t* filename);
```
- **Purpose:** Open/create a file for writing
- **Default:** Can be used for pre-opened streams

```cpp
virtual void Close();
```
- **Purpose:** Close the stream opened by Open method

```cpp
virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position);
```
- **Purpose:** Seek to a position in the stream
- **Required for:** Archive creation and updates
- **Default:** Can be used when passing to Iarchive extract method
- **Returns:** `S_OK` on success, error code otherwise

```cpp
virtual HRESULT SetSize(UInt64 size);
```
- **Purpose:** Preallocate file size
- **Required for:** Archive updates
- **Default:** This is enough in all cases (?)

```cpp
virtual HRESULT Mkdir(const wchar_t* dirname);
```
- **Purpose:** Create directory
- **Required for:** Extracting

```cpp
virtual HRESULT SetMode(const wchar_t* path, UInt32 mode);
```
- **Purpose:** Set file permissions
- **Required for:** Restoring files permission after extraction

```cpp
virtual HRESULT SetTime(const wchar_t* filename, UInt32 time);
```
- **Purpose:** Set file timestamp
- **Required for:** Restoring file mtime after extraction

---

### `Lib` Class

Main class for loading the 7-Zip library and querying supported formats.

#### Constructor/Destructor

```cpp
Lib();
~Lib();
```

#### Methods

##### `load()`
```cpp
bool load(const wchar_t* libname);
```
- **Purpose:** Load the 7-Zip DLL
- **Parameters:**
  - `libname`: Path to the 7z.dll/7z.so file (use `SEVENZIPDLL` macro for default)
- **Returns:** `true` on success, `false` on failure
- **Example:**
  ```cpp
  sevenzip::Lib lib;
  if (!lib.load(SEVENZIPDLL)) {
      wprintf(L"Error: %s\n", lib.getLoadMessage());
  }
  ```

##### `unload()`
```cpp
void unload();
```
- **Purpose:** Unload the 7-Zip library
- **Note:** Called automatically by destructor
- **Note** Can be used to load another library.

##### `isLoaded()`
```cpp
bool isLoaded();
```
- **Purpose:** Check if library is loaded
- **Returns:** `true` if loaded, `false` otherwise

##### `getLoadMessage()`
```cpp
wchar_t* getLoadMessage();
```
- **Purpose:** Get error message from last `load()` attempt
- **Returns:** Wide string containing error message
- **Note:** Result may reside in a statically allocated buffer that is overwritten by subsequent calls

##### `getVersion()`
```cpp
unsigned getVersion();
```
- **Purpose:** Get version of the loaded 7-Zip library
- **Returns:** Version number as unsigned integer with major in upper 16bits and minor in lower 16bits

##### `getNumberOfFormats()`
```cpp
int getNumberOfFormats();
```
- **Purpose:** Get number of supported archive formats
- **Returns:** Number of formats, -1 on error

##### `getFormatName()`
```cpp
wchar_t* getFormatName(int index);
```
- **Purpose:** Get name of format at given index
- **Parameters:**
  - `index`: Format index (0 to `getNumberOfFormats()-1`)
- **Returns:** Format name (e.g., "7z", "zip")
- **Note:** Result may reside in a statically allocated buffer that is overwritten by subsequent calls

##### `getFormatExtensions()`
```cpp
wchar_t* getFormatExtensions(int index);
```
- **Purpose:** Get file extensions for format
- **Parameters:**
  - `index`: Format index
- **Returns:** Space-separated list of extensions
- **Note:** Result may reside in a statically allocated buffer that is overwritten by subsequent calls

##### `getFormatUpdatable()`
```cpp
bool getFormatUpdatable(int index);
```
- **Purpose:** Check if format supports archive creation
- **Parameters:**
  - `index`: Format index
- **Returns:** `true` if format is updatable

##### `getFormatByExtension()`
```cpp
int getFormatByExtension(const wchar_t* ext);
```
- **Purpose:** Find format index by file extension
- **Parameters:**
  - `ext`: File extension (e.g., "zip", "7z")
- **Returns:** Format index or -1 if not found

##### `getFormatBySignature()`
```cpp
int getFormatBySignature(Istream& stream, const wchar_t* ext = nullptr);
```
- **Purpose:** Detect format by reading file signature
- **Parameters:**
  - `stream`: Input stream positioned at archive start
  - `ext`: Optional file extension hint for ambiguous signatures
- **Returns:** Format index or -1 if not recognized

---

### `Iarchive` Class

Class for reading and extracting archives.

#### Constructor/Destructor

```cpp
Iarchive();
~Iarchive();
```

#### Methods

##### `open()` - Basic
```cpp
HRESULT open(Lib& lib, Istream& istream, 
             const wchar_t* filename, int formatIndex = -1);
```
- **Purpose:** Open an archive for reading
- **Parameters:**
  - `lib`: Loaded Lib instance
  - `istream`: Input stream (can be pre-opened or unopened)
  - `filename`: Archive filename (used for format detection)
  - `formatIndex`: 
    - `>=  0`: Force specific format
    - `== -1`: Auto-detect by extension then signature (default)
    - `<= -2`: Auto-detect by signature only
- **Returns:** `S_OK` on success, error code otherwise
- **Note:** Istream must be seekable for most formats
- **Note:** Istream can be pre-opened to open single-volume archive
- **Note:** Istream.Clone should be implemented to open multi-volume archive

##### `open()` - With Password
```cpp
HRESULT open(Lib& lib, Istream& istream,
             const wchar_t* filename, const wchar_t* password, 
             int formatIndex = -1);
```
- **Purpose:** Open password-protected archive
- **Parameters:** Same as above plus:
  - `password`: Archive password
- **Returns:** `S_OK` on success, error code otherwise

##### `close()`
```cpp
void close();
```
- **Purpose:** Close the archive
- **Note:** Called automatically by destructor

##### `extract()` - Full Archive
```cpp
HRESULT extract(Ostream& ostream, int index = -1);
```
- **Purpose:** Extract archive contents
- **Parameters:**
  - `ostream`: Output stream for writing files
  - `index`:
    - `== -1`: Extract all items (default)
    - `>=  0`: Extract specific item by index
- **Returns:** `S_OK` on success, error code otherwise
- **Note:** For multi-file extraction, `ostream.Open()` will be called for each file
- **Note:** For single-file extraction pre-opened stream can be used

##### `extract()` - With Password
```cpp
HRESULT extract(Ostream& ostream, const wchar_t* password, int index = -1);
```
- **Purpose:** Extract password-protected content
- **Parameters:** Same as above plus password

##### `getNumberOfItems()`
```cpp
int getNumberOfItems();
```
- **Purpose:** Get number of items in archive
- **Returns:** Item count, -1 on error

##### `getItemPath()`
```cpp
const wchar_t* getItemPath(int index);
```
- **Purpose:** Get path/name of item
- **Parameters:**
  - `index`: Item index (0 to `getNumberOfItems()-1`)
- **Returns:** Item path as wide string
- **Note:** Result may reside in a statically allocated buffer that is overwritten by subsequent calls

##### `getItemSize()`
```cpp
UInt64 getItemSize(int index);
```
- **Purpose:** Get uncompressed size of item
- **Parameters:**
  - `index`: Item index
- **Returns:** Size in bytes

##### `getItemMode()`
```cpp
UInt32 getItemMode(int index);
```
- **Purpose:** Get file mode/permissions
- **Parameters:**
  - `index`: Item index
- **Returns:** Mode bits

##### `getItemTime()`
```cpp
UInt32 getItemTime(int index);
```
- **Purpose:** Get file timestamp
- **Parameters:**
  - `index`: Item index
- **Returns:** Timestamp value

##### `getItemIsDir()`
```cpp
bool getItemIsDir(int index);
```
- **Purpose:** Check if item is a directory
- **Parameters:**
  - `index`: Item index
- **Returns:** `true` if directory

#### Advanced Property Methods

##### Archive Properties

```cpp
int getNumberOfProperties();
HRESULT getPropertyInfo(int propIndex, PROPID& propId, VARTYPE& propType);
HRESULT getStringProperty(PROPID propId, const wchar_t*& propValue);
HRESULT getBoolProperty(PROPID propId, bool& propValue);
HRESULT getIntProperty(PROPID propId, UInt32& propValue);
HRESULT getWideProperty(PROPID propId, UInt64& propValue);
HRESULT getTimeProperty(PROPID propId, UInt32& propValue);
```
- **Purpose:** Low-level access to archive properties
- **Note:** See 7-Zip SDK headers CPP/7zip/PropID.h and CPP/Common/MyWindows.h for PROPID and VARTYPE values

##### Item Properties

```cpp
int getNumberOfItemProperties();
HRESULT getItemPropertyInfo(int propIndex, PROPID& propId, VARTYPE& propType);
HRESULT getStringItemProperty(int index, PROPID propId, const wchar_t*& propValue);
HRESULT getBoolItemProperty(int index, PROPID propId, bool& propValue);
HRESULT getIntItemProperty(int index, PROPID propId, UInt32& propValue);
HRESULT getWideItemProperty(int index, PROPID propId, UInt64& propValue);
HRESULT getTimeItemProperty(int index, PROPID propId, UInt32& propValue);
```
- **Purpose:** Low-level access to item properties
- **Note:** See 7-Zip SDK headers CPP/7zip/PropID.h and CPP/Common/MyWindows.h for PROPID and VARTYPE values

---

### `Oarchive` Class

Class for creating and updating archives.

#### Constructor/Destructor

```cpp
Oarchive();
~Oarchive();
```

#### Methods

##### `open()` - Basic
```cpp
HRESULT open(Lib& lib, Istream& istream, Ostream& ostream,
             const wchar_t* filename, int formatIndex = -1);
```
- **Purpose:** Open/create archive for writing
- **Parameters:**
  - `lib`: Loaded Lib instance
  - `istream`: Input stream for reading files to add
  - `ostream`: Output stream for writing archive
  - `filename`: Archive filename
  - `formatIndex`: 
    - `>=  0`: Use specific format
    - `== -1`: Auto-detect by extension (default)
- **Returns:** `S_OK` on success, error code otherwise

##### `open()` - With Password
```cpp
HRESULT open(Lib& lib, Istream& istream, Ostream& ostream,
             const wchar_t* filename, const wchar_t* password, 
             int formatIndex = -1);
```
- **Purpose:** Create password-protected archive
- **Parameters:** Same as above plus:
  - `password`: Archive password

##### `close()`
```cpp
void close();
```
- **Purpose:** Close the archive
- **Note:** Called automatically by destructor

##### `addItem()`
```cpp
void addItem(const wchar_t* pathname);
```
- **Purpose:** Add file or directory to archive
- **Parameters:**
  - `pathname`: Path to file or directory to add
- **Note:** Call this multiple times to add multiple items

##### `update()`
```cpp
HRESULT update();
```
- **Purpose:** Write the archive with all added items
- **Returns:** `S_OK` on success, error code otherwise
- **Note:** This performs the actual compression and archive creation
- **Note** Clears internal list of items created by addItem method calls

##### Property Setters

```cpp
HRESULT setStringProperty(const wchar_t* name, const wchar_t* value);
HRESULT setBoolProperty(const wchar_t* name, bool value);
HRESULT setIntProperty(const wchar_t* name, UInt32 value);
HRESULT setWideProperty(const wchar_t* name, UInt64 value);
HRESULT setEmptyProperty(const wchar_t* name);
```
- **Purpose:** Set compression and archive properties
- **Common Properties:**
  - `"x"`: Compression level (0-9)
  - `"s"`: Solid mode (true/false)
  - `"m"`: Compression method ("lzma")
  - `"mt"`: Number of threads
- **Returns:** `S_OK` on success, error code otherwise

---

### Utility Functions

#### `getMessage()`
```cpp
wchar_t* getMessage(HRESULT hr);
```
- **Purpose:** Get human-readable error message for HRESULT
- **Parameters:**
  - `hr`: HRESULT error code
- **Returns:** Error message as wide string
- **Note:** Result may reside in a statically allocated buffer that is overwritten by subsequent calls

#### `getResult()`
```cpp
HRESULT getResult(bool noerror);
```
- **Purpose:** Convert boolean to HRESULT
- **Parameters:**
  - `noerror`: Boolean value
- **Returns:** `S_OK` if `noerror` is true, otherwise HRESULT for system error code or `E_FAIL` if system error code not defined

#### `getVersion()`
```cpp
UInt32 getVersion();
```
- **Purpose:** Get 7-Zip SDK version
- **Returns:** Version number (major << 16 | minor)
- **Note:** This is the 7-Zip SDK version used to build libsevenzip.

#### String Conversion Functions

```cpp
wchar_t* fromBytes(const char* str);
wchar_t* fromBytes(wchar_t* buffer, size_t size, const char* str);
```
- **Purpose:** Convert multi-byte string to wide string
- **Note:** First variant uses static buffer (1024 wchars)

```cpp
char* toBytes(const wchar_t* str);
char* toBytes(char* buffer, size_t size, const wchar_t* str);
```
- **Purpose:** Convert wide string to multi-byte string
- **Note:** First variant uses static buffer (1024*sizeof(wchar_t) bytes)

---

## Usage Examples

### Example 1: Loading Library and Listing Formats

```cpp
#include "sevenzip.h"
#include <iostream>

int main() {
    sevenzip::Lib lib;
    
    // Load 7-Zip library
    if (!lib.load(SEVENZIPDLL)) {
        std::wcerr << L"Failed to load: " << lib.getLoadMessage() << std::endl;
        return 1;
    }
    
    std::wcout << L"7-Zip version: " << lib.getVersion() << std::endl;
    std::wcout << L"Supported formats:\n";
    
    // List all supported formats
    int numFormats = lib.getNumberOfFormats();
    for (int i = 0; i < numFormats; i++) {
        std::wcout << L"  " << lib.getFormatName(i) 
                   << L" (" << lib.getFormatExtensions(i) << L")"
                   << (lib.getFormatUpdatable(i) ? L" [writable]" : L"")
                   << std::endl;
    }
    
    return 0;
}
```

### Example 2: Listing Archive Contents

```cpp
class MyIstream : public sevenzip::Istream {
    FILE* f;
public:
    MyIstream() : f(nullptr) {}
    
    HRESULT Open(const wchar_t* filename) override {
        f = _wfopen(filename, L"rb");
        return f ? S_OK : E_FAIL;
    }
    
    void Close() override {
        if (f) { fclose(f); f = nullptr; }
    }
    
    HRESULT Read(void* data, UInt32 size, UInt32& processed) override {
        processed = (UInt32)fread(data, 1, size, f);
        return S_OK;
    }
    
    HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position) override {
        _fseeki64(f, offset, origin);
        position = _ftelli64(f);
        return S_OK;
    }
};

int main() {
    sevenzip::Lib lib;
    lib.load(SEVENZIPDLL);
    
    MyIstream istream;
    sevenzip::Iarchive archive;
    
    // Open archive
    HRESULT hr = archive.open(lib, istream, L"test.7z");
    if (hr != S_OK) {
        std::wcerr << L"Error: " << sevenzip::getMessage(hr) << std::endl;
        return 1;
    }
    
    // List contents
    int count = archive.getNumberOfItems();
    std::wcout << L"Archive contains " << count << L" items:\n";
    
    for (int i = 0; i < count; i++) {
        std::wcout << L"  " << archive.getItemPath(i)
                   << L" (" << archive.getItemSize(i) << L" bytes)"
                   << (archive.getItemIsDir(i) ? L" [DIR]" : L"")
                   << std::endl;
    }
    
    return 0;
}
```

### Example 3: Extracting Archive

```cpp
class MyOstream : public sevenzip::Ostream {
    FILE* f;
    std::wstring basePath;
public:
    MyOstream(const wchar_t* base) : f(nullptr), basePath(base) {}
    
    HRESULT Open(const wchar_t* filename) override {
        std::wstring fullPath = basePath + L"\\" + filename;
        f = _wfopen(fullPath.c_str(), L"wb");
        return f ? S_OK : E_FAIL;
    }
    
    void Close() override {
        if (f) { fclose(f); f = nullptr; }
    }
    
    HRESULT Write(const void* data, UInt32 size, UInt32& processed) override {
        processed = (UInt32)fwrite(data, 1, size, f);
        return S_OK;
    }
    
    HRESULT Mkdir(const wchar_t* dirname) override {
        std::wstring fullPath = basePath + L"\\" + dirname;
        _wmkdir(fullPath.c_str());
        return S_OK;
    }
};

int main() {
    sevenzip::Lib lib;
    lib.load(SEVENZIPDLL);
    
    MyIstream istream;
    MyOstream ostream(L"C:\\output");
    sevenzip::Iarchive archive;
    
    // Open and extract
    if (archive.open(lib, istream, L"test.7z") == S_OK) {
        HRESULT hr = archive.extract(ostream);
        if (hr == S_OK) {
            std::wcout << L"Extracted successfully\n";
        }
    }
    
    return 0;
}
```

### Example 4: Creating Archive

```cpp
class FileIstream : public sevenzip::Istream {
    FILE* f;
public:
    HRESULT Open(const wchar_t* filename) override {
        f = _wfopen(filename, L"rb");
        return f ? S_OK : E_FAIL;
    }
    
    void Close() override {
        if (f) { fclose(f); f = nullptr; }
    }
    
    HRESULT Read(void* data, UInt32 size, UInt32& processed) override {
        processed = (UInt32)fread(data, 1, size, f);
        return S_OK;
    }
    
    UInt64 GetSize(const wchar_t* filename) override {
        struct _stat64 st;
        if (_wstat64(filename, &st) == 0)
            return st.st_size;
        return 0;
    }
    
    bool IsDir(const wchar_t* filename) const override {
        struct _stat64 st;
        return (_wstat64(filename, &st) == 0) && (st.st_mode & _S_IFDIR);
    }
};

int main() {
    sevenzip::Lib lib;
    lib.load(SEVENZIPDLL);
    
    FileIstream istream;
    MyOstream ostream(L"");
    sevenzip::Oarchive archive;
    
    // Create new archive
    if (archive.open(lib, istream, ostream, L"output.7z") == S_OK) {
        // Set compression properties
        archive.setIntProperty(L"x", 5);  // Compression level 5
        archive.setBoolProperty(L"mt", true);  // Multi-threaded
        
        // Add files
        archive.addItem(L"file1.txt");
        archive.addItem(L"file2.txt");
        archive.addItem(L"folder\\");
        
        // Write archive
        HRESULT hr = archive.update();
        if (hr == S_OK) {
            std::wcout << L"Archive created successfully\n";
        }
    }
    
    return 0;
}
```

---

## Error Handling

### HRESULT Codes

Common return values:

- `S_OK` (0x00000000): Success
- `S_FALSE` (0x00000001): Operation completed with false result
- `E_FAIL` (0x80004005): General failure
- `E_ABORT` (0x80004004): Operation aborted
- `E_NOTIMPL` (0x80004001): Not implemented
- `E_NOINTERFACE` (0x80004002): Interface not supported
- `E_NOTSUPPORTED` (0x80004001): Operation not supported
- `E_NEEDPASSWORD` (0x80040001): Password required

### Checking Results

```cpp
HRESULT hr = archive.open(lib, istream, L"test.7z");
if (hr != S_OK) {
    wprintf(L"Error: %s\n", sevenzip::getMessage(hr));
}
```

### Exception Safety

The library does not throw exceptions. All errors are reported via HRESULT return values. Destructors automatically clean up resources.

---

## Platform Support

### Windows
- Uses `7z.dll`
- Native wide character support
- Requires Visual C++ or compatible compiler

### Linux / macOS
- Uses `7z.so`
- Wide character support via platform libraries
- Requires GCC or Clang with C++11

### Type Definitions

Platform-specific types are defined for compatibility:

**Windows:**
- `HRESULT`: `long`
- `PROPID`: `unsigned long`
- `VARTYPE`: `unsigned short`

**Linux:** / **macOS**
- `HRESULT`: `Int32`
- `PROPID`: `UInt32`
- `VARTYPE`: `UInt16`

---

## Best Practices

1. **Always check return values**: Test HRESULT codes from all operations
2. **Implement necessary stream methods**: Based on your use case (reading vs writing)
3. **Use format auto-detection**: Unless you need to force a specific format
4. **Set compression properties before update**: Properties must be set before calling `update()`
5. **Close archives explicitly**: While destructors handle cleanup, explicit closing is clearer
6. **Handle passwords securely**: Don't hardcode passwords in production code
7. **Check stream capabilities**: Ensure your streams support required operations (e.g., Seek for most archives)

---

## Troubleshooting

### Problem: Library fails to load
- **Solution**: Ensure 7z.dll/7z.so is in the system path (PATH, LD_LIBRARY_PATH, DYLD_LIBRARY_PATH)
- Check `getLoadMessage()` for specific error

### Problem: Library interface type mismatch
- **Reason**: Linux/macOS versions of 7z.so v.23+ are incompatible with previous versions
- **Symptom**: `"Library interface type mismatch"` message from `getLoadMessage()`
- **Solution**: Change the used 7z.so or rebuild libsevenzip with correct 7-Zip SDK.

### Problem: Archive won't open
- **Solution**: Verify format is supported and stream is seekable
- Try format auto-detection instead of forcing format

### Problem: Extract fails
- **Solution**: Ensure output stream implements required methods (Open, Write, Mkdir)
- Check password if archive is encrypted

### Problem: Create archive fails
- **Solution**: Verify format is updatable (`getFormatUpdatable()`)
- Ensure input stream implements file metadata methods (IsDir, GetMode, GetTime)

---

## Additional Resources

- 7-Zip SDK Documentation: https://www.7-zip.org/sdk.html
- 7-Zip Source Repository: https://github.com/ip7z/7zip
- Lib7zip C++ Wrapper Library 7-Zip: https://github.com/stonewell/lib7zip
- Libsevenzip Source Repository: https://github.com/ooa64/libsevenzip
- Libsevenzip additional tests: https://github.com/ooa64/tclsevenzip

---

## License

See LICENSE file for library licensing information.

7-Zip is licensed under GNU LGPL. This wrapper library follows its own license terms.
