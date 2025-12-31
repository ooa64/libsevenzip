#include <fstream>
#include <sstream>
#include <iostream>
#include "sevenzip.h"

#include <sys/stat.h>

using namespace std;
using namespace sevenzip;

struct Compressstream: public Istream, private std::ifstream {

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
};

struct Outputstream_preopen: public Ostream {

    Outputstream_preopen(ofstream& stream) : stream(&stream) {}

    virtual HRESULT Write(const void* data, UInt32 size, UInt32& processed) override {
        stream->write((const char*)data, size);
        processed = size;
        return getResult(stream->is_open() && !stream->bad());
    };

    virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position) override {
        stream->clear();
        stream->seekp(offset, static_cast<ios_base::seekdir>(origin));
        position = stream->tellp();
        return getResult(stream->is_open() && !stream->bad());
    };

    ofstream* stream;
};

struct Inputstream_preopen : public Istream {

    Inputstream_preopen(ifstream& stream) : stream(&stream) {}

    virtual HRESULT Read(void* data, UInt32 size, UInt32& processed) override {
        stream->read((char*)data, size);
        processed = size;
        return getResult(stream->is_open() && !stream->bad());
    };

    virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position) override {
        stream->clear();
        stream->seekg(offset, static_cast<ios_base::seekdir>(origin));
        position = stream->tellg();
        return getResult(stream->is_open() && !stream->bad());
    };

    ifstream* stream;
};

struct Extractstream_preopen : public Ostream {

    Extractstream_preopen(ostream& stream) : stream(&stream) {}

    virtual HRESULT Write(const void* data, UInt32 size, UInt32& processed) override {
        stream->write((const char*)data, size);
        processed = size;
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
        ofstream fstream("temps/example8.7z", ios::binary);
        Oarchive a(l);
        Compressstream s;
        Outputstream_preopen o(fstream);
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
        stringstream sstream;
        ifstream fstream("temps/example8.7z", ios::binary);
        Iarchive a(l);
        Inputstream_preopen s(fstream);
        Extractstream_preopen x(sstream);
        hr = a.open(s, nullptr);
        wcout << "open : " << getMessage(hr) << "\n";
        hr = a.extract(x, 0);
        result = (int)sstream.str().size();
        wcout << "extract: " << hr << " " << getMessage(hr) << "\n";
        wcout << "extracted size: " << result << " expected " << expected<< "\n";
        wcout << "extracted data: " << fromBytes(sstream.str().substr(0,16).c_str()) << "\n";
    }
    if (hr != S_OK)
        return 1;

    if (result == expected) {
        wcout << "TEST PASSED\n";
    }
    return 0;
}
