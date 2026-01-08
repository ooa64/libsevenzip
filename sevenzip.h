#ifndef SEVENZIP_H
#define SEVENZIP_H

#define LIBSEVENZIP_VER_MAJOR 1
#define LIBSEVENZIP_VER_MINOR 0

#include "C/7zTypes.h"

#ifdef _WIN32
#define SEVENZIPDLL L"7z.dll"
typedef long HRESULT;
typedef unsigned long PROPID;
typedef unsigned short VARTYPE;
#else
#define SEVENZIPDLL L"7z.so"
typedef Int32 HRESULT;
typedef UInt32 PROPID;
typedef UInt16 VARTYPE;
#endif

#ifndef S_OK
#define S_OK           ((HRESULT)0x00000000L)
#define S_FALSE        ((HRESULT)0x00000001L)
#define E_NOTIMPL      ((HRESULT)0x80004001L)
#define E_NOINTERFACE  ((HRESULT)0x80004002L)
#define E_ABORT        ((HRESULT)0x80004004L)
#define E_FAIL         ((HRESULT)0x80004005L)
#endif
#define E_NOTSUPPORTED ((HRESULT)0x80004001L)
#define E_NEEDPASSWORD ((HRESULT)0x80040001L)

namespace sevenzip {

    const unsigned Version = ((LIBSEVENZIP_VER_MAJOR << 16) | LIBSEVENZIP_VER_MINOR);

    // To be redefined by the user of the library

    // Input stream interface
    // Used to read input archive in the Iarchive class
    // Used to read input files in the Oarchive class
    // Passed in unopened state to Iarchive and Oarchive methods

    struct Istream {

        virtual HRESULT Read(void* data, UInt32 size, UInt32& processed) = 0;

        // Should return false for a preopened stream
        virtual HRESULT Open(const wchar_t* /*filename*/) { return S_FALSE; };
        virtual void Close() {};

        // Used by the open handler
        virtual HRESULT Seek(Int64 /*offset*/, UInt32 /*origin*/, UInt64& /*position*/) { return S_FALSE; };

        // Looks like unused at this time, but left for possible future use
        virtual UInt64 GetSize(const wchar_t* /*filename*/) const { return 0; };

        // Used by the update handler
        virtual bool IsDir(const wchar_t* /*filename*/) const { return false; };
        virtual UInt32 GetMode(const wchar_t* /*filename*/) const { return 0; };
        virtual UInt32 GetTime(const wchar_t* /*filename*/) const { return 0; };

        // Used by open multivolume handler
        virtual Istream* Clone() const { return nullptr; };
        
        virtual ~Istream() = default;
    };

    // Output stream interface
    // Used to write extracted files in the Iarchive class
    // Used to write output archive in the Oarchive class
    // Passed in unopened state to Iarchive and Oarchive methods

    struct Ostream {

        virtual HRESULT Write(const void* data, UInt32 size, UInt32& processed) = 0;

        // Should return false for a preopened stream
        virtual HRESULT Open(const wchar_t* /*filename*/) { return S_FALSE; };
        virtual void Close() {};

        // Used by update handler
        virtual HRESULT Seek(Int64 /*offset*/, UInt32 /*origin*/, UInt64& /*position*/) { return S_FALSE; };
        virtual HRESULT SetSize(UInt64 /*size*/) { return S_FALSE; };

        // Used by extract handler
        virtual HRESULT Mkdir(const wchar_t* /*dirname*/) { return S_FALSE; };
        virtual HRESULT SetMode(const wchar_t* /*path*/, UInt32 /*mode*/) { return S_FALSE; };
        virtual HRESULT SetTime(const wchar_t* /*filename*/, UInt32 /*time*/) { return S_FALSE; };

        virtual ~Ostream() = default;
    };
};

namespace sevenzip {

    // Library
    
    class Lib {
    
    public:

        Lib();
        ~Lib();

        bool load(const wchar_t* libname);
        void unload();
        bool isLoaded();
        wchar_t* getLoadMessage(); // lib.load() error message
        unsigned getVersion();

        int getNumberOfFormats();
        wchar_t* getFormatName(int index);
        wchar_t* getFormatExtensions(int index);
        bool getFormatUpdatable(int index);
        int getFormatByExtension(const wchar_t* ext);
        int getFormatBySignature(Istream& stream);

    private:

        class Impl;
        Impl* pimpl;
        friend class Iarchive;
        friend class Oarchive;
    };

    // Archive reading/extracting class

    class Iarchive {

    public:

        Iarchive();
        ~Iarchive();

        // istream can be preopened in the case of singlevolume archives
        // istream seek position must be at the beginning of the archive

        // formatIndex >  -1 : force format
        // formatIndex == -1 : detect format by extension and then by signature
        // formatIndex <  -1 : detect format by signature

        HRESULT open(Lib& lib, Istream& istream,
                const wchar_t* filename, int formatIndex = -1);
        HRESULT open(Lib& lib, Istream& istream,
                const wchar_t* filename, const wchar_t* password, int formatIndex = -1);

        void close();

        // ostream can be preopened in the case of single item extraction (index > -1)

        HRESULT extract(Ostream& ostream, int index = -1);
        HRESULT extract(Ostream& ostream, const wchar_t* password, int index = -1);

        // archive items listing

        int getNumberOfItems();
        const wchar_t* getItemPath(int index);
        UInt64 getItemSize(int index);
        UInt32 getItemMode(int index);
        UInt32 getItemTime(int index);
        bool getItemIsDir(int index);

        // lowlevel routines, CPP/7zip/PropID.h and CPP/Common/MyWindows.h can be useful

        int getNumberOfProperties();
        HRESULT getPropertyInfo(int propIndex, PROPID& propId, VARTYPE& propType);
        HRESULT getStringProperty(PROPID propId, const wchar_t*& propValue);
        HRESULT getBoolProperty(PROPID propId, bool& propValue);
        HRESULT getIntProperty(PROPID propId, UInt32& propValue);
        HRESULT getWideProperty(PROPID propId, UInt64& propValue);
        HRESULT getTimeProperty(PROPID propId, UInt32& propValue);

        int getNumberOfItemProperties();
        HRESULT getItemPropertyInfo(int propIndex, PROPID& propId, VARTYPE& propType);
        HRESULT getStringItemProperty(int index, PROPID propId, const wchar_t*& propValue);
        HRESULT getBoolItemProperty(int index, PROPID propId, bool& propValue);
        HRESULT getIntItemProperty(int index, PROPID propId, UInt32& propValue);
        HRESULT getWideItemProperty(int index, PROPID propId, UInt64& propValue);
        HRESULT getTimeItemProperty(int index, PROPID propId, UInt32& propValue);

    private:

        class Impl;
        Impl* pimpl;
    };

    // Archive creating/compressing class

    class Oarchive {

    public:

        Oarchive();
        ~Oarchive();

        // ostream can be preopened

        HRESULT open(Lib& lib, Istream& istream, Ostream& ostream,
                const wchar_t* filename, int formatIndex = -1);
        HRESULT open(Lib& lib, Istream& istream, Ostream& ostream,
                const wchar_t* filename, const wchar_t* password, int formatIndex = -1);

        void close();

        void addItem(const wchar_t* pathname);

        HRESULT update();

        HRESULT setStringProperty(const wchar_t* name, const wchar_t* value);
        HRESULT setBoolProperty(const wchar_t* name, bool value);
        HRESULT setIntProperty(const wchar_t* name, UInt32 value);
        HRESULT setWideProperty(const wchar_t* name, UInt64 value);
        HRESULT setEmptyProperty(const wchar_t* name);

    private:

        class Impl;
        Impl* pimpl;
    };

    wchar_t* getMessage(HRESULT hr);
    HRESULT getResult(bool noerror);
    UInt32 getVersion();

    wchar_t *fromBytes(const char* str); // static buffer 1024 wchar_ts
    wchar_t *fromBytes(wchar_t* buffer, size_t size, const char* str);

    char *toBytes(const wchar_t* str); // static buffer 1024*sizeof(wchar_t) chars
    char *toBytes(char* buffer, size_t size, const wchar_t* str);
};

#endif // SEVENZIP_H
