#include <fstream>
#include <sstream>
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

struct itemsstream: public Istream, private std::ifstream {

    virtual HRESULT Open(const wchar_t* path) override {
        open(U2F(path), ios::binary);
        return getResult(is_open());
    }

    virtual void Close() override {
        close();
    } 

    virtual HRESULT Read(void* data, UInt32 size, UInt32* processed) override {
        read((char*)data, size);
        if (processed)
            *processed = (unsigned)gcount();
        return getResult(is_open() && !bad());
    };
};

struct outputstream_preopen: public Ostream {

    outputstream_preopen(ofstream& stream) : stream(&stream) {}

    virtual HRESULT Open(const wchar_t* /*path*/) override { return S_OK; };
    virtual void Close() override {};

    virtual HRESULT Write(const void* data, UInt32 size, UInt32* processed) override {
        stream->write((const char*)data, size);
        if (processed)
            *processed = size;
        return getResult(stream->is_open() && !stream->bad());
    };

    virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64* position) override {
        stream->clear();
        stream->seekp(offset, static_cast<ios_base::seekdir>(origin));
        if (position)
            *position = stream->tellp();
        return getResult(stream->is_open() && !stream->bad());
    };

    ofstream* stream;
};

struct inputstream_preopen : public Istream {

    inputstream_preopen(ifstream& stream) : stream(&stream) {}

    virtual HRESULT Open(const wchar_t* /*path*/) override { return S_FALSE; };
    virtual void Close() override {};

    virtual HRESULT Read(void* data, UInt32 size, UInt32* processed) override {
        stream->read((char*)data, size);
        if (processed)
            *processed = size;
        return getResult(stream->is_open() && !stream->bad());
    };

    virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64* position) override {
        stream->clear();
        stream->seekg(offset, static_cast<ios_base::seekdir>(origin));
        if (position)
            *position = stream->tellg();
        return getResult(stream->is_open() && !stream->bad());
    };

    ifstream* stream;
};

struct extractstream_preopen : public Ostream {

    extractstream_preopen(ostream& stream) : stream(&stream) {}

    virtual HRESULT Open(const wchar_t* /*path*/) override { return S_OK; };
    virtual void Close() override {};

    virtual HRESULT Write(const void* data, UInt32 size, UInt32* processed) override {
        stream->write((const char*)data, size);
        if (processed)
            *processed = size;
        return getResult(!stream->bad());
    };

    ostream* stream;
};

int main() {
    wcout << "Create/extract simple archive using preopen streams\n\n";

    Lib l;
    if (!l.load(SEVENZIPDLL)) {
        wcerr << l.getLoadMessage() << "\n";
        return 1;
    }

    HRESULT hr;
    int result = 0;
    int expected = 0;
    {
        struct stat st;
        if (stat("temps/example8.txt", &st) == 0)
            expected = st.st_size;
    }
    {
        Oarchive a(l);
        ofstream fstream(L"temps/example8.7z", ios::binary);
        itemsstream s;
        outputstream_preopen o(fstream);
        hr = a.open(s, o, nullptr);
        wcout << "open : " << getMessage(hr) << "\n";
        a.addItem(L"temps/example8.txt");
        hr = a.update();
        wcout << "update: " << hr << " " << getMessage(hr) << "\n";
		fstream.close();
    }
    if (hr != S_OK)
        return 1;
    {
        Iarchive a(l);
        stringstream sstream;
        ifstream fstream(L"temps/example8.7z", ios::binary);
        inputstream_preopen s(fstream);
        extractstream_preopen x(sstream);
        hr = a.open(s, nullptr);
        wcout << "open : " << getMessage(hr) << "\n";
        hr = a.extract(x, 0);
        wcout << "extract: " << hr << " " << getMessage(hr) << "\n";
		cout << "extracted size: " << sstream.str().size() << "\n";
		cout << "extracted data: " << sstream.str().substr(0,16) << "\n";
        result = (int)sstream.str().size();
    }
    if (hr != S_OK)
        return 1;

    if (result == expected) {
        cout << "TEST PASSED\n";
    }
    return 0;
}
