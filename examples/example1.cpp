#include <fstream>
#include <iostream>
#include "sevenzip.h"

using namespace std;
using namespace sevenzip;

struct inputstream: public Istream, public std::ifstream {

    virtual HRESULT Open(const wchar_t*) {return S_FALSE;}
    virtual void Close() {}

    virtual HRESULT Read(void* data, UInt32 size, UInt32* processed) {
        read((char*)data, size);
        if (processed)
            *processed = (unsigned)gcount();
        return getResult(!bad());
    };

    virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64* position) {
        clear();
        seekg(offset, static_cast<ios_base::seekdir>(origin));
        if (position)
            *position = tellg();
        return getResult(!bad());
    };
}; 

int main() {
    wcout << "Library functions\n\n";

    Lib l;
    if (!l.load(SEVENZIPDLL)) {
        wcout << l.getLoadMessage() << "\n";
        return 1;
    }

    int f1 = l.getFormatByExtension(L"7z");
    wcout << "format by ext : " << l.getFormatName(f1) << "\n";

    inputstream i;
    i.open("temps/example1.7z", ios::binary);
    int f2 = l.getFormatBySignature(i);
    i.close();
    wcout << "format by sign : " << l.getFormatName(f2) << "\n";

    if (f1 == f2 && f1 >= 0) {
        wcout << "TEST PASSED\n";
    }
    return 0;
}
