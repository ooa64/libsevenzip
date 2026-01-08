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
- `ASAN=1` - with address sanitizer
- `DEBUG=1` - with debug info
- `DEBUG_IMPL=1` - with callbacks trace
