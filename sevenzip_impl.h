#ifndef SEVENZIP_IMPL_H
#define SEVENZIP_IMPL_H

#include "CPP/7zip/MyVersion.h"
#if MY_VER_MAJOR < 15
#error outdated 7zip
#endif

#include "CPP/Common/Common.h"
#include "CPP/Common/MyCom.h"
#include "CPP/Common/MyBuffer.h"
#include "CPP/Common/MyString.h"

#include "CPP/7zip/ICoder.h"
#include "CPP/7zip/IPassword.h"
#include "CPP/7zip/Archive/IArchive.h"

#ifndef _WIN32
typedef void * HMODULE;
#endif

#include "sevenzip_compat.h"
#include "sevenzip.h"

namespace sevenzip {

    class CInStream Z7_final :
        public IInStream,
        public CMyUnknownImp {

        Z7_COM_UNKNOWN_IMP_1(IInStream)

    public:

        STDMETHOD(Read)(void* data, UInt32 size, UInt32* processedSize) throw() Z7_override Z7_final;
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) throw() Z7_override Z7_final;

        // NOTE: istream is owned by caller unless cloned is true
        CInStream(Istream* istream, bool cloned = false);
        virtual ~CInStream();

        HRESULT Open(const wchar_t* filename);
        void Close();

        bool IsDir(const wchar_t* pathname);
        UInt64 GetSize(const wchar_t* pathname);
        UInt32 GetMode(const wchar_t* pathname);
        UInt32 GetTime(const wchar_t* pathname);

    private:

        Istream* istream;
        bool cloned;
    };

    class COutStream Z7_final :
        public IOutStream,
        public CMyUnknownImp {

    public:

        Z7_COM_UNKNOWN_IMP_1(IOutStream)

    public:

        STDMETHOD(Write)(const void* data, UInt32 size, UInt32* processedSize) throw() Z7_override Z7_final;
        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) throw() Z7_override Z7_final;
        STDMETHOD(SetSize)(UInt64 size) throw() Z7_override Z7_final;

        // NOTE: ostream is owned by caller
        COutStream(Ostream* ostream);
        virtual ~COutStream();

        HRESULT Open(const wchar_t* filename);
        void Close();

        HRESULT Mkdir(const wchar_t* dirname);
        HRESULT SetMode(const wchar_t* pathname, UInt32 mode);
        HRESULT SetTime(const wchar_t* pathname, UInt32 time);

    private:

        Ostream* ostream;
    };


    class COpenCallback Z7_final :
        public IArchiveOpenCallback,
        public IArchiveOpenVolumeCallback,
        public IArchiveOpenSetSubArchiveName,
        public ICryptoGetTextPassword,
        public CMyUnknownImp {

    public:

        Z7_COM_UNKNOWN_IMP_4(
            IArchiveOpenCallback,
            IArchiveOpenVolumeCallback,
            IArchiveOpenSetSubArchiveName,
            ICryptoGetTextPassword)

    public:

        STDMETHOD(SetTotal) (const UInt64* files, const UInt64* bytes) throw() Z7_override Z7_final;
        STDMETHOD(SetCompleted) (const UInt64* files, const UInt64* bytes) throw() Z7_override Z7_final;

        STDMETHOD(GetProperty)(PROPID propID, PROPVARIANT* value) throw() Z7_override Z7_final;
        STDMETHOD(GetStream)(const wchar_t* name, IInStream** inStream) throw() Z7_override Z7_final;

        STDMETHOD(SetSubArchiveName)(const wchar_t* name) throw() Z7_override Z7_final;

        STDMETHOD(CryptoGetTextPassword)(BSTR* password) throw() Z7_override Z7_final;

        COpenCallback(Istream* istream, const wchar_t* pathname, const wchar_t* password);
        virtual ~COpenCallback();
        const wchar_t *Password() const;

    private:

        Istream* istream;
        UString pathname;
        UString password;
        bool passworddefined;
        UString subarchivename;
        bool subarchivemode;
    };


    class CExtractCallback Z7_final :
        public IArchiveExtractCallback,
        public ICryptoGetTextPassword,
        public CMyUnknownImp {

        Z7_COM_UNKNOWN_IMP_2(
            IArchiveExtractCallback,
            ICryptoGetTextPassword)

    public:

        STDMETHOD(SetTotal)(UInt64 size) throw() Z7_override Z7_final;
        STDMETHOD(SetCompleted)(const UInt64* completeValue) throw() Z7_override Z7_final;

        STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode) throw() Z7_override Z7_final;
        STDMETHOD(PrepareOperation)(Int32 askExtractMode) throw() Z7_override Z7_final;
        STDMETHOD(SetOperationResult)(Int32 resultEOperationResult) throw() Z7_override Z7_final;

        STDMETHOD(CryptoGetTextPassword) (BSTR* password) throw() Z7_override Z7_final;

        CExtractCallback(Ostream* ostream, IInArchive* archive, const wchar_t* password);
        virtual ~CExtractCallback();

    private:

        CMyComPtr<ISequentialOutStream> outstream;
        IInArchive* archive;
        UString password;
        bool passworddefined;
        int index;
    };


    class CUpdateCallback Z7_final :
        public IArchiveUpdateCallback2,
        public ICryptoGetTextPassword2,
        public CMyUnknownImp {

    public:

        Z7_COM_UNKNOWN_IMP_2(
            IArchiveUpdateCallback2,
            ICryptoGetTextPassword2)

    public:

        STDMETHOD(SetTotal)(UInt64 size) throw() Z7_override Z7_final;
        STDMETHOD(SetCompleted)(const UInt64* completeValue) throw() Z7_override Z7_final;

        STDMETHOD(GetUpdateItemInfo)(UInt32 index,
                Int32* newData, Int32* newProperties, UInt32* indexInArchive) throw() Z7_override Z7_final;
        STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT* value) throw() Z7_override Z7_final;
        STDMETHOD(GetStream)(UInt32 index, ISequentialInStream** inStream) throw() Z7_override Z7_final;
        STDMETHOD(SetOperationResult)(Int32 operationResult) throw() Z7_override Z7_final;
        STDMETHOD(GetVolumeSize)(UInt32 index, UInt64* size) throw() Z7_override Z7_final;
        STDMETHOD(GetVolumeStream)(UInt32 index, ISequentialOutStream** volumeStream) throw() Z7_override Z7_final;

        STDMETHOD(CryptoGetTextPassword2)(Int32* passwordIsDefined, BSTR* password) throw() Z7_override Z7_final;

        CUpdateCallback(Istream* istream, const wchar_t* password);
        virtual ~CUpdateCallback();

        CObjectVector<UString> items;

    private:

        CMyComPtr<ISequentialInStream> instream;
        UString password;
        bool passworddefined;
    };


    class Lib::Impl {

    public:

        Impl();
        ~Impl();

        bool load(const wchar_t* libname);
		void unload();
        bool isLoaded() const;
        wchar_t* getLoadMessage();
        unsigned getVersion();

        int getNumberOfMethods();
        wchar_t* getMethodName(int index);
        // bool getMethodIsEncoder(int index);

        int getNumberOfFormats();
        wchar_t* getFormatExtensions(int index);
        wchar_t* getFormatName(int index);
        bool getFormatUpdatable(int index);
        int getFormatByExtension(const wchar_t* ext);
        int getFormatBySignature(Istream* stream, const wchar_t* ext);

        // for internal use
        GUID getFormatGUID(int index);
        bool isExtensionSupported(int index, const wchar_t* ext);
        UString getStringProperty(int propIndex, PROPID propID);
        bool checkInterfaceType() const;

        Func_CreateObject CreateObjectFunc = nullptr;
        Func_GetNumberOfMethods GetNumberOfMethods = nullptr;
        Func_GetNumberOfFormats GetNumberOfFormats = nullptr;
        Func_GetMethodProperty GetMethodProperty = nullptr;
        Func_GetHandlerProperty GetHandlerProperty = nullptr;
        Func_GetHandlerProperty2 GetHandlerProperty2 = nullptr;
        Func_GetModuleProp GetModuleProp = nullptr;

    private:

        HMODULE lib = nullptr;
        void* GetProcAddress(const char* proc);

        void _unload();

        wchar_t loadMessage[128] = { L'\0' };
        wchar_t lastMethodName[128] = { L'\0' };
        wchar_t lastFormatName[128] = { L'\0' };
        wchar_t lastFormatExtensions[128] = { L'\0' };
    };


    class Iarchive::Impl {

    public:

        Impl();
        ~Impl();

        HRESULT open(Lib::Impl* libimpl, Istream* istream,
                const wchar_t* filename, const wchar_t* password, int formatIndex);

        void close();

        HRESULT extract(Ostream* ostream, const wchar_t* password, int index);

        int getNumberOfItems();
        const wchar_t* getItemPath(int index);
        UInt64 getItemSize(int index);
        UInt32 getItemMode(int index);
        UInt32 getItemTime(int index);
        bool getItemIsDir(int index);

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

        CMyComPtr<IInStream> instream;
        CMyComPtr<IInArchive> inarchive;
        CMyComPtr<IArchiveOpenCallback> opencallback;
        int formatIndex = -1;

        wchar_t lastItemPath[1024] = { L'\0' };
        wchar_t lastStringProperty[1024] = { L'\0' };
    };


    class Oarchive::Impl {

    public:

        Impl();
        ~Impl();

        HRESULT open(Lib::Impl* libimpl, Istream* istream, Ostream* ostream,
                const wchar_t* filename, const wchar_t* password, int formatIndex);

        void close();

        void addItem(const wchar_t* pathname);

        HRESULT update();

        HRESULT setStringProperty(const wchar_t* name, const wchar_t* value);
        HRESULT setBoolProperty(const wchar_t* name, bool value);
        HRESULT setIntProperty(const wchar_t* name, UInt32 value);
        HRESULT setWideProperty(const wchar_t* name, UInt64 value);
        HRESULT setEmptyProperty(const wchar_t* name);

    private:

        CMyComPtr<IOutStream> outstream;
        CMyComPtr<IOutArchive> outarchive;
        CMyComPtr<IArchiveUpdateCallback2> updatecallback;
        int formatIndex = -1;
    };

#define COPYACHARS(_d_,_s_) (wcsncpy((_d_),(as2us(_s_)),(sizeof(_d_)/sizeof(_d_[0])-1)))
#define COPYWCHARS(_d_,_s_) (wcsncpy((_d_),(_s_),(sizeof(_d_)/sizeof(_d_[0])-1)))

#ifdef USE_UNICODE_FSTRING
#define as2us(_s_) (MultiByteToUnicodeString((AString)(_s_), CP_UTF8))
#define us2as(_s_) (UnicodeStringToMultiByte((UString)(_s_), CP_UTF8))
#else
#define as2us(_s_) fs2us(_s_)
#define us2as(_s_) us2fs(_s_)
#endif

#define CINSTREAM(_i_) (static_cast<CInStream*>((void*)(_i_)))
#define COUTSTREAM(_i_) (static_cast<COutStream*>((void*)(_i_)))
#define COPENCALLBACK(_i_) (static_cast<COpenCallback*>((void*)(_i_)))
#define CUPDATECALLBACK(_i_) (static_cast<CUpdateCallback*>((void*)(_i_)))

}

#endif // SEVENZIP_IMPL_H
