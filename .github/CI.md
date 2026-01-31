# CI/CD Configuration

This repository uses GitHub Actions for continuous integration and testing across multiple platforms.

## Build Matrix

The CI system tests the library with:

### Platforms
- **Linux** (Ubuntu Latest)
- **macOS** (Latest)
- **Windows** (Latest)

### Build Systems
- **Make/NMAKE** - Traditional makefile-based builds
- **CMake** - Cross-platform CMake builds

### Compilers
- **Linux**: GCC and Clang
- **macOS**: Apple Clang
- **Windows**: MSVC (x64 and Win32)

### 7-Zip Dependencies
The CI tests with both official and extended 7-Zip libraries:
- **ip7z/7zip** - Official 7-Zip source repository
- **mcmilk/7-Zip-zstd** - 7-Zip with additional codec support (Zstandard, Brotli, LZ4, etc.)

## Workflow Triggers

The CI workflow runs on:
- Push to `main`, `master`, or `develop` branches
- Pull requests to `main`, `master`, or `develop` branches
- Manual workflow dispatch

## Build Configurations

### Linux
- Make builds with GCC and Clang
- CMake builds with Debug and Release configurations
- Ninja generator for faster builds

### macOS
- Make builds with system Clang
- CMake builds with Debug and Release configurations
- Ninja generator

### Windows
- NMAKE builds with x64 and Win32 platforms
- CMake builds with Debug and Release, x64 and Win32
- Visual Studio generator

## Artifacts

Build artifacts are uploaded for each successful build:
- Static libraries (`libsevenzip.a` / `sevenzip.lib`)
- Test executables
- Example programs
- Retention: 7 days

## Tests

All builds run the test suite automatically:
- Unit tests via the `tests` executable
- Tests are run on the platform they were built for
- Build fails if tests fail

## Status Badge

Add this badge to your README to show CI status:

```markdown
![CI](https://github.com/YOUR_USERNAME/libsevenzip/workflows/CI/badge.svg)
```

## Local Testing

To test locally with the same dependency structure:

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/libsevenzip.git
cd libsevenzip

# Clone one of the 7-Zip dependencies
git clone https://github.com/ip7z/7zip.git ../7zip
# OR
git clone https://github.com/mcmilk/7-Zip-zstd.git ../7zip

# Build with Make
make SEVENZIPSRC=../7zip
make tests SEVENZIPSRC=../7zip
./tests/tests

# Or build with CMake
cmake -B build -DSEVENZIPSRC=../7zip
cmake --build build
cd build && ctest --output-on-failure
```

## Troubleshooting

### Dependency Issues
If you see errors about missing 7-Zip sources, ensure:
1. The 7-Zip repository is cloned correctly
2. The `SEVENZIPSRC` path is correct
3. The 7-Zip repository structure matches expectations (C/ and CPP/ directories)

### Platform-Specific Issues

**Linux:**
- Ensure build-essential is installed: `sudo apt-get install build-essential`
- For CMake builds: `sudo apt-get install cmake ninja-build`

**macOS:**
- Install Xcode Command Line Tools: `xcode-select --install`
- For CMake builds: `brew install cmake ninja`

**Windows:**
- Visual Studio 2019 or later required
- Run builds from Developer Command Prompt

## Matrix Strategy

The CI uses GitHub Actions matrix strategy to efficiently test multiple configurations:
- 2 7-Zip dependencies
- 3 platforms
- 2 build systems per platform
- Multiple compiler/configuration combinations

This results in comprehensive coverage with parallel execution for faster feedback.
