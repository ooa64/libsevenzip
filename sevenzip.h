#ifndef SEVENZIP_H
#define SEVENZIP_H

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
#define S_OK          ((HRESULT)0x00000000L)
#define S_FALSE       ((HRESULT)0x00000001L)
#define E_NOTIMPL     ((HRESULT)0x80004001L)
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#define E_ABORT       ((HRESULT)0x80004004L)
#define E_FAIL        ((HRESULT)0x80004005L)
#endif

namespace sevenzip {

    // To be redefined by the user of the library 

    struct Istream {

        virtual ~Istream() = default;
    
        virtual HRESULT Open(const wchar_t* filename) = 0;
        virtual HRESULT Read(void* data, UInt32 size, UInt32* processed) = 0;
        virtual void Close() = 0;

        // Used by the open handler
        virtual HRESULT Seek(Int64 /*offset*/, UInt32 /*origin*/, UInt64* /*position*/) { return S_FALSE; };

        // Used by open multivolume handler
        virtual Istream* Clone() const { return nullptr; };
        virtual const wchar_t* Path() const { return L""; };
        
        // Used by the update handler
        virtual bool IsDir(const wchar_t* /*filename*/) const { return false; };
        //virtual UInt64 GetSize(const wchar_t* /*filename*/) const { return 1; };
        virtual UInt32 GetMode(const wchar_t* /*filename*/) const { return 0; };
        virtual UInt32 GetTime(const wchar_t* /*filename*/) const { return 0; };
    };

    struct Ostream {

        virtual ~Ostream() = default;

        virtual HRESULT Open(const wchar_t* filename) = 0;
        virtual HRESULT Write(const void* data, UInt32 size, UInt32* processed) = 0;
        virtual void Close() = 0;

        // Used by update handler
        virtual HRESULT Seek(Int64 /*offset*/, UInt32 /*origin*/, UInt64* /*position*/) { return S_FALSE; };
        virtual HRESULT SetSize(UInt64 /*size*/) { return S_FALSE; };

        // Used by extract handler
        virtual HRESULT Mkdir(const wchar_t* /*dirname*/) { return S_FALSE; };
        virtual HRESULT SetMode(const wchar_t* /*path*/, UInt32 /*mode*/) { return S_FALSE; };
        virtual HRESULT SetTime(const wchar_t* /*filename*/, UInt32 /*time*/) { return S_FALSE; };
    };
};

namespace sevenzip {

    // Library
    
    class Lib {
    
    public:

        Lib();
        ~Lib();

        bool load(const wchar_t* libname);
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


    class Iarchive {

    public:

        Iarchive() = delete;
        Iarchive(Lib& lib);
        ~Iarchive();

        HRESULT open(Istream* istream,
                const wchar_t* filename, int formatIndex = -1);
        HRESULT open(Istream* istream,
                const wchar_t* filename, const wchar_t* password, int formatIndex = -1);

        void close();

        int getNumberOfItems();
        const wchar_t* getItemPath(int index);
        UInt64 getItemSize(int index);
        UInt32 getItemMode(int index);
        Int64 getItemTime(int index);
        bool getItemIsDir(int index);

        HRESULT extract(Ostream* ostream, const wchar_t* directory, int index = -1);
        HRESULT extract(Ostream* ostream, const wchar_t* directory, const wchar_t* password, int index = -1);

        // lowlevel routines, CPP/7zip/PropID.h and CPP/Common/MyWindows.h can be useful

        int getNumberOfProperties();
        const wchar_t* getPropertyInfo(int propIndex, PROPID& propId, VARTYPE& propType);
        HRESULT getStringProperty(PROPID propId, const wchar_t*& propValue);
        HRESULT getBoolProperty(PROPID propId, bool& propValue);
        HRESULT getIntProperty(PROPID propId, UInt32& propValue);
        HRESULT getWideProperty(PROPID propId, UInt64& propValue);

        int getNumberOfItemProperties();
        const wchar_t* getItemPropertyInfo(int propIndex, PROPID& propId, VARTYPE& propType);
        HRESULT getStringItemProperty(int index, PROPID propId, const wchar_t*& propValue);
        HRESULT getBoolItemProperty(int index, PROPID propId, bool& propValue);
        HRESULT getIntItemProperty(int index, PROPID propId, UInt32& propValue);
        HRESULT getWideItemProperty(int index, PROPID propId, UInt64& propValue);

    private:

        class Impl;
        Impl* pimpl;
    };


    class Oarchive {

    public:

        Oarchive() = delete;
        Oarchive(Lib& lib);
        ~Oarchive();

        HRESULT open(Istream* istream, Ostream* ostream,
                const wchar_t* filename, int formatIndex = -1);
        HRESULT open(Istream* istream, Ostream* ostream,
                const wchar_t* filename, const wchar_t* password, int formatIndex = -1);

        void close();

        void addItem(const wchar_t* pathname);

        HRESULT update();

    private:

        class Impl;
        Impl* pimpl;
    };

    wchar_t* getMessage(HRESULT hr);
    HRESULT getResult(bool noerror);
    UInt32 getVersion();
};

#endif
