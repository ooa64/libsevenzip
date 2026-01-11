#include <fstream>
#include <iostream>
#include "sevenzip.h"

using namespace std;
using namespace sevenzip;

struct Inputstream: public Istream, public std::ifstream {

    virtual HRESULT Read(void* data, UInt32 size, UInt32& processed) {
        read((char*)data, size);
        processed = (UInt32)gcount();
        return getResult(!bad());
    };

    virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position) {
        clear();
        seekg(offset, static_cast<ios_base::seekdir>(origin));
        position = tellg();
        return getResult(!bad());
    };
}; 

int main() {
    setlocale(LC_ALL, "");
    wcout << "Library functions\n\n";

    Lib l;
    if (!l.load(SEVENZIPDLL)) {
        wcout << l.getLoadMessage() << "\n";
        return 1;
    }

    int f1 = l.getFormatByExtension(L"7z");
    wcout << "format by ext : " << l.getFormatName(f1) << "\n";

    Inputstream i;
    i.open("temps/example1.7z", ios::binary);
    int f2 = l.getFormatBySignature(i);
    i.close();
    wcout << "format by sign : " << l.getFormatName(f2) << "\n";

    if (f1 == f2 && f1 >= 0) {
        wcout << "TEST PASSED\n";
    }
    return 0;
}
