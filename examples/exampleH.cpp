#include <filesystem>
#include <fstream>
#include <iostream>
#include "sevenzip.h"

using namespace std;
using namespace sevenzip;

struct Inputstream: public Istream, public std::ifstream {

    virtual HRESULT Open(const wchar_t* filename) override {
        filesystem::path fn(filename);
        open(fn, ios::binary);
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

struct Compressstream: public Inputstream {

    virtual HRESULT Open(const wchar_t* filename) override {
        wcout << "Compressing " << filename << "\n";
        return Inputstream::Open(filename);
    };

    virtual Istream* Clone() const override {
        return new Compressstream();
    };

    virtual bool IsDir(const wchar_t* pathname) override {
        filesystem::path pn(pathname);
        return filesystem::is_directory(pn);
    };

    virtual UInt64 GetSize(const wchar_t* pathname) override {
        filesystem::path pn(pathname);
        return std::filesystem::file_size(pn);
    };

    virtual UInt32 GetMode(const wchar_t* /*pathname*/) override {
        // TODO: implement file mode conversion to POSIX
        // filesystem::path pn(pathname);
        // auto perm = filesystem::status(pn).permissions();
        // wcout << "Getting perm for " << pathname << " : " << perm << "\n";
        return 0;
    };

    virtual UInt32 GetTime(const wchar_t* /*pathname*/) override {
        // TODO: implement file time conversion to POSIX
        // filesystem::path pn(pathname);
        // auto mtime = std::filesystem::last_write_time(pn);
        // wcout << "Getting mtime for " << pathname << " : " << mtime << "\n";
        return 0;
    };
};

struct Outputstream: public Ostream, public std::ofstream {

    virtual HRESULT Open(const wchar_t* filename) override {
        filesystem::path fn(filename);
        open(fn, ios::binary);
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

struct Extractstream: public Outputstream {

    Extractstream(const wchar_t* basepath) : basepath(basepath) {};

    virtual HRESULT Open(const wchar_t* filename) override {
        wcout << "Extracting " << filename << "\n";
        filesystem::path fn(fullname(filename));
        filesystem::create_directories(fn.parent_path());
        return Outputstream::Open(filename);
    };

    virtual HRESULT Mkdir(const wchar_t* dirname) override {
        wcout << "Creating " << dirname << "\n";
        filesystem::path dn(fullname(dirname));
        filesystem::create_directories(dn);
        return S_OK;
    };

    virtual HRESULT SetMode(const wchar_t* pathname, UInt32 /*mode*/) override {
        // TODO: implement file mode conversion from POSIX
        filesystem::path pn(fullname(pathname));
        filesystem::perms perm = filesystem::perms::owner_read
                | filesystem::perms::owner_write
                | filesystem::perms::group_read
                | filesystem::perms::others_read;
        filesystem::permissions(pn, perm, filesystem::perm_options::replace);
        return S_OK; 
    };
    
    virtual HRESULT SetTime(const wchar_t* pathname, UInt32 /*time*/) override {
        // TODO: implement file time conversion from POSIX
        const filesystem::file_time_type mtime = filesystem::file_time_type::clock::now(); 
        filesystem::path pn(fullname(pathname));
        filesystem::last_write_time(pn, mtime);
        return S_OK;
    };

private:

    wstring fullname(const wchar_t* filename) const {
        return basepath.empty() ? filename : basepath + L"/" + filename;
    };

    wstring basepath;
};

static const wchar_t * const usage =
L"7-Zip example application\n\n"
L"Usage: example.exe [a | l | x] archive.ext [fileName ...]\n"
L"Examples:\n"
L"  example.exe a archive.7z f1.txt f2.txt  : Add two files to archive.7z\n"
L"  example.exe x archive.7z dir  : eXtract files from archive.7z\n"
L"  example.exe l archive.7z   : List contents of archive.7z\n";

int main(int argc, char** argv) {
    setlocale(LC_ALL, "");

    if (argc < 3) {
        wcout << "\n" << usage;
        return 1;
    }

    Lib l;
    if (!l.load(SEVENZIPDLL)) {
        wcout  << "\n" << l.getLoadMessage() << "\n";
        return 1;
    }

    wcout << "7-Zip " << (getVersion() >> 16) << "." << (getVersion() & 0xffff) << " example "
            << "(" SEVENZIPDLL " " << (l.getVersion() >> 16) << "." << (l.getVersion() & 0xffff) << ")\n\n";

    HRESULT hr;
    switch (argv[1][0]) {

        case 'a': {
            Oarchive a;
            Compressstream c;
            Outputstream o;
            hr = a.open(l, c, o, fromBytes(argv[2]));
            if (hr == S_OK) {
                for (int i = 3; i < argc; i++) {
                    a.addItem(fromBytes(argv[i]));
                }
                hr = a.update();
            }
            break;
        }

        case 'l': {
            Iarchive a;
            Inputstream s;
            hr = a.open(l, s, fromBytes(argv[2]));
            if (hr == S_OK) {
                int n = a.getNumberOfItems();
                for (int i = 0; i < n; i++) {
                    wcout << a.getItemSize(i) << "  " << a.getItemPath(i) << "\n";
                }
            }
            break;
        }

        case 'x': {
            Iarchive a;
            Inputstream s;
            hr = a.open(l, s, fromBytes(argv[2]));
            if (hr == S_OK) {
                Extractstream e(argc > 3 ? fromBytes(argv[3]) : L"");
                hr = a.extract(e);
            }
            break;
        }

        default:
            wcout << usage;
            return 1;
    }
    wcout << "\nHRESULT " << hex << hr << " : " << getMessage(hr) << "\n\n";
    return 0;
}
