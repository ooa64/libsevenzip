#include <iostream>
#include "sevenzip.h"

#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <sys/utime.h>
#define MAIN(_c_,_v_) wmain(int _c_, wchar_t** _v_)
#define SETLOCALE
#define U2F(_s_) (_s_)
#define F2U(_s_) (_s_)
#define FOPEN(_h_,_f_,_m_) (_wfopen_s((_h_),(_f_),(_m_)))
#define MKDIR(_d_,_m_) (_wmkdir(_d_))
#define CHMOD(_f_,_m_) (_wchmod((_f_),(_m_)))
#define STAT(_p_,_b_) (_wstat((_p_),(_b_)))
#define UTIME(_p_,_t_) (_wutime((_p_),(_t_)))
#define STRUCT_STAT struct _stat
#define STRUCT_UTIMBUF struct _utimbuf
#else
#include <utime.h>
#include <locale>
#include <codecvt>
std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
#define MAIN(_c_,_v_) main(int _c_, char** _v_)
#define SETLOCALE (setlocale(LC_ALL, ""))
#define U2F(_s_) (convert.to_bytes(_s_).c_str())
#define F2U(_s_) (convert.from_bytes(_s_).c_str())
#define FOPEN(_h_,_f_,_m_) (fopen_s((_h_),U2F(_f_),U2F(_m_)))
#define MKDIR(_d_,_m_) (mkdir(U2F(_d_),(_m_)))
#define CHMOD(_f_,_m_) (chmod(U2F(_f_),(_m_)))
#define STAT(_p_,_b_) (stat(U2F(_p_),(_b_)))
#define UTIME(_p_,_t_) (utime(U2F(_p_),(_t_)))
#define STRUCT_STAT struct stat
#define STRUCT_UTIMBUF struct utimbuf
#endif

using namespace std;
using namespace sevenzip;

struct Inputstream: public Istream {

    virtual HRESULT Open(const wchar_t* filename) override {
        this->path = filename;
        return getResult(FOPEN(&this->file, filename, L"rb") == 0);
    }

    virtual void Close() override {
        if (this->file)
            fclose(this->file);
        //this->file = nullptr;
        path.clear();
    }

    virtual HRESULT Read(void* data, UInt32 size, UInt32& processed) override {
		size_t count = fread(data, 1, size, this->file);
        processed = (UInt32)count;
        return getResult(count >= 0);
    };

    virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position) override {
		int result = fseek(this->file, (long)offset, origin);
        position = ftell(this->file);
        return getResult(result == 0);
    };

    virtual Istream* Clone() const override {
        return new Inputstream();
    };

    virtual const wchar_t* Path() const override {
        return path.c_str();
    };

    std::wstring path; 
    FILE* file = nullptr;
};

struct Compressstream: public Inputstream {

    virtual HRESULT Open(const wchar_t* filename) override {
        wcout << "Compressing " << filename << "\n";
        return Inputstream::Open(filename);
    };

    virtual Istream* Clone() const override {
        return new Compressstream();
    };

    virtual bool IsDir(const wchar_t* pathname) const override {
        STRUCT_STAT s;
        if (STAT(pathname, &s) == 0)
            return s.st_mode & S_IFDIR;
        return 0;
    };

    virtual UInt32 GetMode(const wchar_t* pathname) const override {
        STRUCT_STAT s;
        if (STAT(pathname, &s) == 0)
            return s.st_mode;
        return 0;
    };

    virtual UInt32 GetTime(const wchar_t* pathname) const override {
        STRUCT_STAT s;
        if (STAT(pathname, &s) == 0)
            return static_cast<UInt32>(s.st_mtime);
        return 0;
    };
};

struct Outputstream: public Ostream {

    virtual HRESULT Open(const wchar_t* filename) override {
        return getResult(FOPEN(&this->file, filename, L"wb") == 0);
    };

    virtual void Close() override {
        if (this->file)
            fclose(this->file);
        //this->file = nullptr;
    };

    virtual HRESULT Write(const void* data, UInt32 size, UInt32& processed) override {
        size_t count = fwrite(data, 1, size, this->file);
        processed = (UInt32)count;
        return getResult(count >= 0);
    };

    virtual HRESULT Seek(Int64 offset, UInt32 origin, UInt64& position) override {
		int result = fseek(this->file, (long)offset, origin);
        position = ftell(this->file);
        return getResult(result == 0);
    };
    
    FILE* file = nullptr;
};

struct Extractstream: public Outputstream {

    Extractstream(const wchar_t* basepath) : basepath(basepath) {};

    virtual HRESULT Open(const wchar_t* filename) override {
        wcout << "Extracting " << filename << "\n";
        return Outputstream::Open(fullname(filename).c_str());
    };

    virtual HRESULT Mkdir(const wchar_t* dirname) override {
        wcout << "Creating " << dirname << "\n";
        (void)MKDIR(fullname(dirname).c_str(), 0755);
        return S_OK;
    };

    virtual HRESULT SetMode(const wchar_t* pathname, UInt32 mode) override {
        (void)CHMOD(fullname(pathname).c_str(), mode);
        return S_OK; 
    };
    
    virtual HRESULT SetTime(const wchar_t* pathname, UInt32 time) override {
        STRUCT_UTIMBUF t;
        t.actime = 0;
        t.modtime = time;
        UTIME(fullname(pathname).c_str(), &t);
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

int MAIN(argc, argv) {
    SETLOCALE;

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
            Oarchive a(l);
            Compressstream c;
            Outputstream o;
            hr = a.open(c, o, F2U(argv[2]));
            if (hr == S_OK) {
                for (int i = 3; i < argc; i++) {
                    a.addItem(F2U(argv[i]));
                }
                hr = a.update();
            }
            break;
        }

        case 'l': {
            Iarchive a(l);
            Inputstream s;
            hr = a.open(s, F2U(argv[2]));
            if (hr == S_OK) {
                int n = a.getNumberOfItems();
                for (int i = 0; i < n; i++) {
                    wcout << a.getItemSize(i) << "  " << a.getItemPath(i) << "\n";
                }
            }
            break;
        }

        case 'x': {
            Iarchive a(l);
            Inputstream s;
            hr = a.open(s, F2U(argv[2]));
            if (hr == S_OK) {
                Extractstream e(argc > 3 ? F2U(argv[3]) : L"");
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
