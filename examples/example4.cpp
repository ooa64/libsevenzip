#include <fstream>
#include <iostream>
#include "sevenzip.h"

#include <sys/stat.h>

using namespace std;
using namespace sevenzip;

struct Inputstream: public Istream, private std::ifstream {

    virtual HRESULT Open(const wchar_t* path) override {
        open(toBytes(path), ios::binary);
        return getResult(is_open());
    }

    virtual void Close() override {
        close();
    } 

    virtual HRESULT Read(void* data, UInt32 size, UInt32& processed) override {
        read((char*)data, size);
        processed = (UInt32)gcount();
        return getResult(is_open() && !bad());
    };

    virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position) override {
        clear();
        seekg(offset, static_cast<ios_base::seekdir>(origin));
        position = tellg();
        return getResult(is_open() && !bad());
    };

    virtual Istream* Clone() const override {
        return new Inputstream();
    };
}; 

int main() {
    wcout << "Open multivolume archive\n\n";

    Lib l;
    if (!l.load(SEVENZIPDLL)) {
        wcerr << l.getLoadMessage() << "\n";
        return 1;
    }

    Iarchive a;
    Inputstream s;
    HRESULT hr = a.open(l, s, L"temps/example4.7z.001");
    wcout << "open : " << getMessage(hr) << "\n";
    wcout << "items :\n";
    int n = a.getNumberOfItems();
    for (int i = 0; i < n; i++) {
        wcout << i+1 << " : " << a.getItemPath(i) << "\n";
    }
    a.close();

    if (n > 0) {
        wcout << "TEST PASSED\n";
    }
    return 0;
}
