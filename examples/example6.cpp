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

    virtual void Close() override{
        close();
    } 

    virtual HRESULT Read(void* data, UInt32 size, UInt32& processed) override {
        read((char*)data, size);
        processed = (UInt32)gcount();
        return getResult(is_open() && !bad());
    };
};

struct Outputstream: public Ostream, private std::ofstream {

    virtual HRESULT Open(const wchar_t* path) override {
        open(toBytes(path), ios::binary);
        return getResult(is_open());
    };

    virtual void Close() override {
        close();
    };

    virtual HRESULT Write(const void* data, UInt32 size, UInt32& processed) override {
        write((const char*)data, size);
        processed = size;
        return getResult(is_open() && !bad());
    };

    virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position) override {
        clear();
        seekp(offset, static_cast<ios_base::seekdir>(origin));
        position = tellp();
        return getResult(is_open() && !bad());
    };
};

int main() {
    wcout << "Create simple archive\n\n";

    Lib l;
    if (!l.load(SEVENZIPDLL)) {
        wcerr << l.getLoadMessage() << "\n";
        return 1;
    }

    Oarchive a(l);
    Inputstream i;
	Outputstream o;
    HRESULT hr = a.open(i, o, L"temps/example6.7z");
    wcout << "open : " << getMessage(hr) << "\n";
    a.addItem(L"temps/example6.txt");
    hr = a.update();
    wcout << "update: " << hr << " " << getMessage(hr) << "\n";    
    a.close();

    if (hr == 0) {
        struct stat st;
        if (stat("temps/example6.7z", &st) == 0 && st.st_size > 32)  
            wcout << "TEST PASSED\n";
    }
    return 0;
}
