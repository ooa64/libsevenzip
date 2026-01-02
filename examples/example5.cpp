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

    virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position) override {
        clear();
        seekg(offset, static_cast<ios_base::seekdir>(origin));
        position = tellg();
        return getResult(is_open() && !bad());
    };
};

struct outputstream: public Ostream, private std::ofstream {

    outputstream(const wchar_t* basepath): basepath(basepath) {};

    virtual HRESULT Open(const wchar_t* filename) override {
        open(toBytes(fullname(filename).c_str()), ios::binary);
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

private:
    
    wstring fullname(const wchar_t* filename) const {
        return basepath.empty() ? filename : basepath + L"/" + filename;
    };

    wstring basepath;
};

int main() {
    wcout << "Extract from simple archive\n\n";

    Lib l;
    if (!l.load(SEVENZIPDLL)) {
        wcerr << l.getLoadMessage() << "\n";
        return 1;
    }

    Iarchive a;
    Inputstream s;
    HRESULT hr = a.open(l, s, L"temps/example5.7z");
    wcout << "open : " << getMessage(hr) << "\n";
    wcout << "items :\n";
    for (int i = 0; i < a.getNumberOfItems(); i++) {
        wcout << i+1 << " : " << a.getItemPath(i) << "\n";
    }
    outputstream o(L"temps");
    hr = a.extract(o);
    wcout << "extract: " << hr << " " << getMessage(hr) << "\n";    
    a.close();

    if (hr == 0) {
        struct stat st;
        if (stat("temps/example.txt", &st) == 0 && st.st_size > 0)  
            wcout << "TEST PASSED\n";
    }
    return 0;
}
