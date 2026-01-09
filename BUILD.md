# libsevenzip

C++ binding to the 7zip dynamic library

### Requirements

- C++ compiler with C++11 support or later
- 7-Zip SDK to build library (p7zip is ok)
- 7-Zip executable to prepare data for examples
- 7-Zip 7z.dll or 7z.so to run examples

### Build on Windows

```
git clone https://github.com/ip7z/7zip
git clone https://github.com/ooa64/libsevenzip
cd libsevenzip
call vcvars64.bat
nmake -f makefile.vc
nmake -f makefile.vc 7zip
nmake -f makefile.vc examples
nmake -f makefile.vc release
```

### Build on Linux

```
git clone https://github.com/ip7z/7zip
git clone https://github.com/ooa64/libsevenzip
cd libsevenzip
make
make 7zip
make examples
make release
```

### Build Options

- `SEVENZIPSRC=path` - path to the 7zip/p7zip source
- `SEVENZIPBIN=executable` - name of the 7zip/p7zip executable (usually 7z or 7zz)
- `ASAN=1` - with address sanitizer
- `DEBUG=1` - with debug info
- `DEBUG_IMPL=1` - with callbacks trace

### Build Targets

- `all` - build library (default)
- `release` - create zipped release
- `examples` - build and test examples (example[0-9])
- `example` - build demo program (example)
- `valgrind` - check memory usage of running examples
- `leaks` - check memory usage of running examples (macOS)
- `tests` - build and run tests
- `7z` - try to build 7zip executable
- `7zip` - try to build 7zip dynamic library
- `clean` - remove temporary objects
- `cleanall` - remove all artifacts created except 7zip and zipped release
