# libsevenzip

C++ binding to the 7zip dynamic library

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
```
