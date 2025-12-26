#include <fstream>
#include <iostream>
#include "sevenzip.h"

#include <sys/stat.h>

#ifdef _WIN32
#define U2F(_s_) (_s_)
#else
#include <locale>
#include <codecvt>
std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
#define U2F(_s_) (convert.to_bytes(_s_).c_str())
#endif

using namespace std;
using namespace sevenzip;

struct Inputstream: public Istream, private std::ifstream {

    virtual HRESULT Open(const wchar_t* path) override {
        open(U2F(path), ios::binary);
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
};

int main() {
    wcout << "Open simple archive\n\n";

    Lib l;
    if (!l.load(SEVENZIPDLL)) {
        wcerr << l.getLoadMessage() << "\n";
        return 1;
    }

    Iarchive a(l);
    Inputstream s;
    HRESULT hr = a.open(s, L"temps/example2.7z");
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
