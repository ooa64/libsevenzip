# libsevenzip

C++ binding to the 7zip dynamic library

[![CI](https://github.com/ooa64/libsevenzip/workflows/CI/badge.svg)](https://github.com/ooa64/libsevenzip/actions/workflows/ci.yml)
[![Quick Check](https://github.com/ooa64/libsevenzip/workflows/Quick%20Check/badge.svg)](https://github.com/ooa64/libsevenzip/actions/workflows/quick-check.yml)
[![Nightly Build](https://github.com/ooa64/libsevenzip/workflows/Nightly%20Build/badge.svg)](https://github.com/ooa64/libsevenzip/actions/workflows/nightly.yml)

[Documentation](DOCUMENTATION.md)

Usage (Windows)
```
call vcvars64.bat
cl -MD example.cpp sevenzip.lib
example l example.exe
```

Usage (Linux)
```
gcc example.cpp -o example -L. -lsevenzip -lstdc++
example l example
```
