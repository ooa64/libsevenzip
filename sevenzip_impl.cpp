#include "CPP/Common/MyWindows.h"
#include "CPP/Common/MyInitGuid.h"

#include "sevenzip_impl.h"

#include "CPP/Common/StringConvert.h"
#include "CPP/Windows/NtCheck.h"
#include "CPP/Windows/PropVariant.h"
#include "CPP/Windows/TimeUtils.h"
#include "CPP/Windows/ErrorMsg.h"

#ifndef _WIN32
#include <dlfcn.h>
#endif

#ifdef DEBUG_IMPL
#   include <iostream>
#   define DEBUGLOG(_x_) (std::wcerr << "DEBUG: " << _x_ << "\n")
#   define UNUSED(_x_) _x_
#else
#   define DEBUGLOG(_x_)
#   define UNUSED(_x_)
#endif

namespace sevenzip {

    static const wchar_t * const kEmptyFileAlias = L"[Content]";

    HRESULT getResult(bool noerror) {
        if (noerror)
            return S_OK;
        DWORD error = ::GetLastError();
#ifdef _WIN32
        if (error == 0)
           error = errno;
#endif
        if (error == 0)
            return E_FAIL;
        return HRESULT_FROM_WIN32(error);
    };

    wchar_t* getMessage(HRESULT hr) {
        static thread_local wchar_t lastMessage[128] = { L'\0' };
#ifdef _WIN32
        if (hr == S_OK)
            COPYWCHARS(lastMessage, L"Success");
        else if (hr == S_FALSE)
            COPYWCHARS(lastMessage, L"False condition");
        else if (hr == E_NOTSUPPORTED)
            COPYWCHARS(lastMessage, L"Not supported");
        else if (hr == E_NEEDPASSWORD)
            COPYWCHARS(lastMessage, L"Need password");
        else 
            COPYWCHARS(lastMessage, NWindows::NError::MyFormatMessage(hr));
#else
        if (hr == S_OK)
            COPYWCHARS(lastMessage, L"success");
        else if (hr == S_FALSE)
            COPYWCHARS(lastMessage, L"false condition");
        else if (hr == E_NOTSUPPORTED)
            COPYWCHARS(lastMessage, L"not supported");
        else if (hr == E_NEEDPASSWORD)
            COPYWCHARS(lastMessage, L"need password");
        else 
            COPYWCHARS(lastMessage, NWindows::NError::MyFormatMessage(hr));
#endif
        return lastMessage;
    };

    UInt32 getVersion() {
        return ((MY_VER_MAJOR << 16) | MY_VER_MINOR);
    };

    wchar_t *fromBytes(const char* str) {
        static wchar_t buffer[1024];
        return fromBytes(buffer, sizeof(buffer)/sizeof(buffer[0]), str);
    };

    wchar_t *fromBytes(wchar_t* buffer, size_t size, const char* str) {
        wcsncpy(buffer, as2us(str), size - 1);
        buffer[size - 1] = L'\0';
        return buffer;
    };

    char *toBytes(const wchar_t* str) {
        static char buffer[1024*sizeof(wchar_t)];
        return toBytes(buffer, sizeof(buffer), str);
    };

    char *toBytes(char* buffer, size_t size, const wchar_t* str) {
        strncpy(buffer, us2as(str), size - 1);
        buffer[size - 1] = '\0';
        return buffer;
    };

    static UString getFilenameExt(const wchar_t* filename) {
        if (!filename)
            return L"";
        UString n(filename);
        int dot = n.ReverseFind_Dot();
        if (dot < 0 || dot < n.ReverseFind_PathSepar())
            return L"";
        return n.Ptr((unsigned)(dot + 1));
    };

    static HRESULT getStringValue(NWindows::NCOM::CPropVariant& prop, UString& propValue) {
        if (prop.vt == VT_EMPTY)
            return S_FALSE;
        if (prop.vt == VT_BSTR)
            propValue = prop.bstrVal;
        else
            return E_FAIL;
        return S_OK;
    };

    static HRESULT getBoolValue(NWindows::NCOM::CPropVariant& prop, bool& propValue) {
        if (prop.vt == VT_EMPTY)
            return S_FALSE;
        if (prop.vt == VT_BOOL)
            propValue = prop.boolVal != VARIANT_FALSE;
        else
            return E_FAIL;
        return S_OK;
    };

    static HRESULT getIntValue(NWindows::NCOM::CPropVariant& prop, UInt32& propValue) {
        if (prop.vt == VT_EMPTY)
            return S_FALSE;
        if (prop.vt == VT_UI1 || prop.vt == VT_I1)
            propValue = prop.bVal;
        else if (prop.vt == VT_UI2 || prop.vt == VT_I2)
            propValue = prop.uiVal;
        else if (prop.vt == VT_UI4 || prop.vt == VT_I4)
            propValue = prop.ulVal;
        else
            return E_FAIL;
        return S_OK;
    };

    static HRESULT getWideValue(NWindows::NCOM::CPropVariant& prop, UInt64& propValue) {
        if (prop.vt == VT_EMPTY)
            return S_FALSE;
        if (prop.vt == VT_UI8 || prop.vt == VT_I8)
            propValue = prop.uhVal.QuadPart;
        else
            return E_FAIL;
        return S_OK;
    };

    static HRESULT getTimeValue(NWindows::NCOM::CPropVariant& prop, UInt32& propValue) {
        if (prop.vt == VT_EMPTY)
            return S_FALSE;
        else if (prop.vt == VT_FILETIME)
            return NWindows::NTime::FileTime_To_UnixTime(prop.filetime, propValue) ? S_OK : S_FALSE;
        else
            return E_FAIL;
    };

    static HRESULT getArchiveStringItemProperty(IInArchive* archive, int index, PROPID propId, UString& propValue) {
        NWindows::NCOM::CPropVariant prop;
        HRESULT hr = archive->GetProperty(index, propId, &prop);
        if (hr != S_OK)
            return hr;
        return getStringValue(prop, propValue);
    };

    static HRESULT getArchiveBoolItemProperty(IInArchive* archive, int index, PROPID propId, bool& propValue) {
        NWindows::NCOM::CPropVariant prop;
        HRESULT hr = archive->GetProperty(index, propId, &prop);
        if (hr != S_OK)
            return hr;
        if (prop.vt == VT_BOOL)
            propValue = prop.boolVal != VARIANT_FALSE;
        else if (prop.vt == VT_EMPTY)
            return S_FALSE;
        else
            return E_FAIL;
        return S_OK;
    };

    static HRESULT getArchiveIntItemProperty(IInArchive* archive, int index, PROPID propId, UInt32& propValue) {
        NWindows::NCOM::CPropVariant prop;
        HRESULT hr = archive->GetProperty(index, propId, &prop);
        if (hr != S_OK)
            return hr;
        return getIntValue(prop, propValue);
    };

    static HRESULT getArchiveWideItemProperty(IInArchive* archive, int index, PROPID propId, UInt64& propValue) {
        NWindows::NCOM::CPropVariant prop;
        HRESULT hr = archive->GetProperty(index, propId, &prop);
        if (hr != S_OK)
            return hr;
        return getWideValue(prop, propValue);
    };

    static HRESULT getArchiveTimeItemProperty(IInArchive* archive, int index, PROPID propId, UInt32& propValue) {
        NWindows::NCOM::CPropVariant prop;
        HRESULT hr = archive->GetProperty(index, propId, &prop);
        if (hr != S_OK)
            return hr;
        return getTimeValue(prop, propValue);
    };

    static HRESULT setProperty(IOutArchive* archive, const wchar_t* name, NWindows::NCOM::CPropVariant prop) {
        CMyComPtr<ISetProperties> setter;
        HRESULT hr = archive->QueryInterface(IID_ISetProperties, (void **)&setter);
        if (setter)
            hr = setter->SetProperties(&name, &prop, 1);
        return hr;
    };

    // streams

    CInStream::CInStream(Istream* istream, bool cloned): istream(istream), cloned(cloned) {
        DEBUGLOG(this << " CInStream " << istream << " " << cloned);
    };

    CInStream::~CInStream() {
        DEBUGLOG(this << " ~CInStream");
        if (cloned && istream)
            delete istream;
    };

    STDMETHODIMP CInStream::Read(void* data, UInt32 size, UInt32* processedSize) throw() {
        DEBUGLOG(this << " CInStream::Read " << size);
        UInt32 dummy = 0;
        return istream ? istream->Read(data, size, processedSize ? *processedSize : dummy) : S_FALSE;
    };

    STDMETHODIMP CInStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) throw() {
        DEBUGLOG(this << " CInStream::Seek " << offset << "/" << seekOrigin);
        UInt64 dummy = 0;
        return istream ? istream->Seek(offset, seekOrigin, newPosition ? *newPosition : dummy) : S_FALSE;
    };

    HRESULT CInStream::Open(const wchar_t* path) {
        DEBUGLOG(this << " CInStream::Open " << path);
        return istream ? istream->Open(path) : S_FALSE;
    };

    void CInStream::Close() {
        DEBUGLOG(this << " CInStream::Close");
        if (istream) istream->Close();
    };

    bool CInStream::IsDir(const wchar_t* pathname) {
        DEBUGLOG(this << " CInStream::IsDir " << pathname);
        return istream ? istream->IsDir(pathname) : false;
    };

    // NOTE: not used at this time, but implemented for possible future use
    UInt64 CInStream::GetSize(const wchar_t* pathname) {
       DEBUGLOG(this << " CInStream::GetSize " << pathname);
       return istream ? istream->GetSize(pathname) : 0;
    }

    UInt32 CInStream::GetTime(const wchar_t* pathname) {
        DEBUGLOG(this << " CInStream::GetTime " << pathname);
        return istream ? istream->GetTime(pathname) : 0;
    };

    UInt32 CInStream::GetMode(const wchar_t* pathname) {
        DEBUGLOG(this << " CInStream::GetMode " << pathname);
        return istream ? istream->GetMode(pathname) : 0;
    };

    UInt32 CInStream::GetAttr(const wchar_t* pathname) {
        DEBUGLOG(this << " CInStream::GetAttr " << pathname);
        return istream ? istream->GetAttr(pathname) : 0;
    };

    COutStream::COutStream(Ostream* ostream): ostream(ostream) {
        DEBUGLOG(this << " COutStream");
    };

    COutStream::~COutStream() {
        DEBUGLOG(this << " ~COutStream");
    };

    STDMETHODIMP COutStream::Write(const void* data, UInt32 size, UInt32* processedSize) throw() {
        DEBUGLOG(this << " COutStream::Write " << size);
        UInt32 dummy = 0;
        return ostream ? ostream->Write(data, size, processedSize ? *processedSize : dummy) : S_FALSE;
    };

    STDMETHODIMP COutStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) throw() {
        DEBUGLOG(this << " COutStream::Seek " << offset << "/" << seekOrigin);
        UInt64 dummy = 0;
        return ostream ? ostream->Seek(offset, seekOrigin, newPosition ? *newPosition : dummy) : S_FALSE;
    };

    STDMETHODIMP COutStream::SetSize(UInt64 size) throw() {
        DEBUGLOG(this << " COutStream::SetSize " << size);
        return ostream ? ostream->SetSize(size) : S_FALSE;
    };

    HRESULT COutStream::Mkdir(const wchar_t* dirname) {
        DEBUGLOG(this << " COutStream::Mkdir " << dirname);
        return ostream ? ostream->Mkdir(dirname) : S_FALSE;
    };
    
    HRESULT COutStream::SetMode(const wchar_t* pathname, UInt32 mode) {
        DEBUGLOG(this << " COutStream::SetMode " << pathname << " " << std::oct << mode << std::dec);
        return ostream ? ostream->SetMode(pathname, mode) : S_FALSE;
    };
    
    HRESULT COutStream::SetAttr(const wchar_t* pathname, UInt32 attr) {
        DEBUGLOG(this << " COutStream::SetAttr " << pathname << " " << std::hex << attr << std::dec);
        return ostream ? ostream->SetAttr(pathname, attr) : S_FALSE;
    };
    
    HRESULT COutStream::SetTime(const wchar_t* pathname, UInt32 time) {
        DEBUGLOG(this << " COutStream::SetTime " << pathname << " " << time);
        return ostream ? ostream->SetTime(pathname, time) : S_FALSE;
    };

    HRESULT COutStream::Open(const wchar_t* filename) {
        DEBUGLOG(this << " COutStream::Open " << filename);
        return ostream ? ostream->Open(filename) : S_FALSE;
    };

    void COutStream::Close() {
        DEBUGLOG(this << " COutStream::Close");
        if (ostream) ostream->Close();
    };

    // callbacks

    COpenCallback::COpenCallback(Istream* istream, const wchar_t* pathname, const wchar_t* password) :
        istream(istream),
        pathname(pathname ? pathname : L""),
        password(password ? password : L""),
        passworddefined(password),
        subarchivename(L""),
        subarchivemode(false) {
        DEBUGLOG(this << " COpenCallback " << istream << " " << (password ? password : L"NULL"));
    };

    COpenCallback::~COpenCallback() {
        DEBUGLOG(this << " ~COpenCallback");
    };

    STDMETHODIMP COpenCallback::SetTotal(const UInt64* UNUSED(files), const UInt64* UNUSED(bytes))  throw() {
        DEBUGLOG(this << " COpenCallback::SetTotal " << (files ? *files : -1) << "/" << (bytes ? *bytes : -1));
        return S_OK;
    };

    STDMETHODIMP COpenCallback::SetCompleted(const UInt64* UNUSED(files), const UInt64* UNUSED(bytes)) throw() {
        DEBUGLOG(this << " COpenCallback::SetCompleted " << (files ? *files : -1) << "/" << (bytes ? *bytes : -1));
        return S_OK;
    };

    STDMETHODIMP COpenCallback::GetProperty(PROPID propID, PROPVARIANT* value) throw() {
        DEBUGLOG(this << " COpenCallback::GetProperty " << propID);
        NWindows::NCOM::CPropVariant prop;
        if (subarchivemode && propID == kpidName) {
            prop = subarchivename;
        }
        else {
            switch (propID) {
            case kpidPath: prop = pathname; break;
            case kpidName: prop = pathname; break;
            case kpidIsDir: prop = istream->IsDir(pathname); break;
            case kpidSize: prop = istream->GetSize(pathname); break;
            case kpidCTime: PropVariant_SetFrom_UnixTime(prop, istream->GetTime(pathname)); break;
            case kpidATime: PropVariant_SetFrom_UnixTime(prop, istream->GetTime(pathname)); break;
            case kpidMTime: PropVariant_SetFrom_UnixTime(prop, istream->GetTime(pathname)); break;
            case kpidAttrib: prop = istream->GetAttr(pathname); break;
            case kpidPosixAttrib: prop = istream->GetMode(pathname); break;
            default: break;
            }
        }
        prop.Detach(value);

        return S_OK;
    };

    STDMETHODIMP COpenCallback::GetStream(const wchar_t* name, IInStream** inStream)  throw() {
        DEBUGLOG(this << " COpenCallback::GetStream " << name << "/" << *inStream);
        *inStream = nullptr;
        pathname = L"";

        if (subarchivemode)
            return S_FALSE;

        auto newIstream = istream->Clone();
        if (!newIstream)
            return E_FAIL;

        CMyComPtr<IInStream> instream(new CInStream(newIstream, true));
        *inStream = instream.Detach();
        pathname = name ? name : L"";

        HRESULT hr = newIstream->Open(name);

        // NOTE: suppress FILE_NOT_FOUND so there is no fail after the last multivolume file
        // DEBUGLOG(this << " COpenCallback::GetStream " << std::hex << hr << " vs " << std::hex << HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
        // return  hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) ? S_FALSE : hr;
        return (hr & 0xffff) == ERROR_FILE_NOT_FOUND ? S_FALSE : hr;
    };

    STDMETHODIMP COpenCallback::SetSubArchiveName(const wchar_t* name) throw() {
        DEBUGLOG(this << " COpenCallback::SetSubArchiveName " << name);
        subarchivemode = true;
        subarchivename = name ? name : L"";
        return S_OK;
    };

    STDMETHODIMP COpenCallback::CryptoGetTextPassword(BSTR* password)  throw() {
        DEBUGLOG(this << " COpenCallback::CryptoGetTextPassword " << passworddefined);
        *password = NULL;
        if (!passworddefined)
            return E_NEEDPASSWORD;
        return StringToBstr(this->password, password);
    };

    const wchar_t* COpenCallback::Password() const {
        return passworddefined ? password.Ptr() : nullptr;
    } 


    CExtractCallback::CExtractCallback(Ostream* ostream, IInArchive* archive, const wchar_t* password) :
            outstream(new COutStream(ostream)),
            archive(archive),
            password(password ? password : L""),
            passworddefined(password != nullptr),
            index(-1) {
        DEBUGLOG(this << " CExtractCallback::CExtractCallback " << archive << " "
                << (password ? password : L"NULL"));
    };

    CExtractCallback::~CExtractCallback() {
        DEBUGLOG(this << " CExtractCallback::~CExtractCallback");
    };

    STDMETHODIMP CExtractCallback::SetTotal(UInt64 UNUSED(size)) throw() {
        DEBUGLOG(this << " CExtractCallback::SetTotal " << size);
        return S_OK;
    };

    STDMETHODIMP CExtractCallback::SetCompleted(const UInt64* UNUSED(completeValue)) throw() {
        DEBUGLOG(this << " CExtractCallback::SetCompleted " << (completeValue ? *completeValue : -1));
        return S_OK;
    };

    STDMETHODIMP CExtractCallback::GetStream(UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode) throw() {
        DEBUGLOG(this << " CExtractCallback::GetStream " << index << " stream " << *outStream << " mode " << askExtractMode);
        *outStream = nullptr;
        this->index = -1;

        if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
            return S_OK;

        if (!outstream)
            return E_FAIL;

        HRESULT hr;
        UString pathname = kEmptyFileAlias;
        hr = getArchiveStringItemProperty(archive, index, kpidPath, pathname);
        if (FAILED(hr))
            return hr;

        bool isdir = false;
        hr = getArchiveBoolItemProperty(archive, index, kpidIsDir, isdir);
        if (FAILED(hr))
            return hr;

        this->index = index;
        if (isdir)
            return COUTSTREAM(outstream)->Mkdir(pathname);

        *outStream = outstream;
        outstream->AddRef();

        hr = COUTSTREAM(outstream)->Open(pathname);
        DEBUGLOG(this << " CExtractCallback::GetStream Open " << pathname.Ptr() << " hr " << hr);
        return FAILED(hr) ? hr : S_OK;
    };

    STDMETHODIMP CExtractCallback::PrepareOperation(Int32 UNUSED(askExtractMode)) throw() {
        DEBUGLOG(this << " CExtractCallback::PrepareOperation " << askExtractMode);
        return S_OK;
    };

    STDMETHODIMP CExtractCallback::SetOperationResult(Int32 operationResult) throw() {
        DEBUGLOG(this << " CExtractCallback::SetOperationResult " << operationResult << " item " << index);
        if (operationResult == NArchive::NExtract::NOperationResult::kOK) {
            if (outstream && index >= 0) {
                COUTSTREAM(outstream)->Close();

                UString pathname = kEmptyFileAlias;
                if (getArchiveStringItemProperty(archive, index, kpidPath, pathname) == S_OK) {

                    bool isdir = false;
                    getArchiveBoolItemProperty(archive, index, kpidIsDir, isdir);

                    UInt32 time = 0;
                    getArchiveTimeItemProperty(archive, index, kpidMTime, time);
                    if (time != 0)
                        COUTSTREAM(outstream)->SetTime(pathname, time);
                    
                    UInt32 attr = 0;
                    getArchiveIntItemProperty(archive, index, kpidAttrib, attr);
                    if (attr != 0)
                        COUTSTREAM(outstream)->SetAttr(pathname, attr & 0x8000 ? attr & 0x7FFF : attr);

                    UInt32 mode = 0;
                    getArchiveIntItemProperty(archive, index, kpidPosixAttrib, mode);
                    if (mode != 0)
                        COUTSTREAM(outstream)->SetMode(pathname, mode);
                    else if (attr & 0x8000)
                        COUTSTREAM(outstream)->SetMode(pathname, (attr >> 16));
                    else if (isdir)
                        COUTSTREAM(outstream)->SetMode(pathname, 0700);

                    DEBUGLOG(this << " CExtractCallback::SetOperationResult set " << pathname.Ptr()
                            << " time " << time
                            << " mode " << std::oct << mode
                            << " attr " << std::hex << attr << std::dec);
                }
            }
            return S_OK;
        }
        if (operationResult == NArchive::NExtract::NOperationResult::kWrongPassword)
            return E_NEEDPASSWORD;
        if (operationResult == NArchive::NExtract::NOperationResult::kUnsupportedMethod)
            return E_NOTSUPPORTED;
        return E_FAIL;
    };

    STDMETHODIMP CExtractCallback::CryptoGetTextPassword(BSTR* password) throw() {
        DEBUGLOG(this << " CExtractCallback::CryptoGetTextPassword " << passworddefined);
        *password = NULL;
        if (!passworddefined)
            return E_NEEDPASSWORD;
        return StringToBstr(this->password, password);
    };


    CUpdateCallback::CUpdateCallback(Istream* istream, const wchar_t* password) :
            instream(new CInStream(istream)),
            password(password ? password : L""),
            passworddefined(password != nullptr) {
        DEBUGLOG(this << " CUpdateCallback " << istream << " " << (password ? password : L"NULL"));
    };

    CUpdateCallback::~CUpdateCallback() {
        DEBUGLOG(this << " ~CUpdateCallback");
    };

    STDMETHODIMP CUpdateCallback::SetTotal(UInt64 UNUSED(size)) throw() {
        DEBUGLOG(this << " CUpdateCallback::SetTotal " << size);
        return S_OK;
    };

    STDMETHODIMP CUpdateCallback::SetCompleted(const UInt64* UNUSED(completeValue)) throw() {
        DEBUGLOG(this << " CUpdateCallback::SetCompleted " << (completeValue ? *completeValue : 0));
        return S_OK;
    };

    STDMETHODIMP CUpdateCallback::GetUpdateItemInfo(UInt32 UNUSED(index),
            Int32* newData, Int32* newProperties, UInt32* indexInArchive) throw() {
        DEBUGLOG(this << " CUpdateCallback::GetUpdateItemInfo " << index);

        if (newData)
            *newData = BoolToInt(true);
        if (newProperties)
            *newProperties = BoolToInt(true);
        if (indexInArchive)
            *indexInArchive = (UInt32)(Int32)-1;

        return S_OK;
    };

    STDMETHODIMP CUpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT* value) throw() {
        DEBUGLOG(this << " CUpdateCallback::GetProperty " << index << " id " << propID);

        // NOTE: alternative implementation without prop variable
        // if (propID == kpidIsAnti) {
        //     NWindows::NCOM::PropVarEm_Set_Bool(value, false);
        // } else {
        //     switch (propID) {
        //     // case kpidPath: value = (BSTR)items[index]; break;
        //     case kpidIsDir: NWindows::NCOM::PropVarEm_Set_Bool(value, false); break;
        //     case kpidSize: NWindows::NCOM::PropVarEm_Set_UInt64(value, 0); break;
        //     case kpidCTime: NWindows::NCOM::PropVarEm_Set_FileTime64_Prec(value, 0, 0); break;
        //     case kpidATime: NWindows::NCOM::PropVarEm_Set_FileTime64_Prec(value, 0, 0); break;
        //     case kpidMTime: NWindows::NCOM::PropVarEm_Set_FileTime64_Prec(value, 0, 0); break;
        //     case kpidAttrib: NWindows::NCOM::PropVarEm_Set_UInt32(value, 32); break;
        //     case kpidPosixAttrib: NWindows::NCOM::PropVarEm_Set_UInt32(value, 0x816d); break;
        //     default: return S_FALSE;
        //     }
        // }

        NWindows::NCOM::CPropVariant prop;
        if (propID == kpidIsAnti) {
            prop = false;
        }
        else {
            if (!instream)
                return E_FAIL;
            switch (propID) {
            case kpidPath: prop = items[index]; break;
            case kpidIsDir: prop = CINSTREAM(instream)->IsDir(items[index]); break;
            case kpidSize: prop = CINSTREAM(instream)->GetSize(items[index]); break;
            case kpidCTime: PropVariant_SetFrom_UnixTime(prop, CINSTREAM(instream)->GetTime(items[index])); break;
            case kpidATime: PropVariant_SetFrom_UnixTime(prop, CINSTREAM(instream)->GetTime(items[index])); break;
            case kpidMTime: PropVariant_SetFrom_UnixTime(prop, CINSTREAM(instream)->GetTime(items[index])); break;
            case kpidAttrib: prop = CINSTREAM(instream)->GetAttr(items[index]); break;
            case kpidPosixAttrib: prop = CINSTREAM(instream)->GetMode(items[index]); break;
            default: break;
            }
        }
        prop.Detach(value);

        return S_OK;
    };

    STDMETHODIMP CUpdateCallback::GetStream(UInt32 index, ISequentialInStream** inStream) throw() {
        DEBUGLOG(this << " CUpdateCallback::GetStream " << index);
        *inStream = nullptr;
        *inStream = instream;
        instream->AddRef();

        HRESULT hr = CINSTREAM(instream)->Open(items[index]);
        return FAILED(hr) ? hr : S_OK;
    };

    STDMETHODIMP CUpdateCallback::SetOperationResult(Int32 UNUSED(operationResult)) throw() {
        DEBUGLOG(this << " CUpdateCallback::SetOperationResult " << operationResult);
        CINSTREAM(instream)->Close();
        return S_OK;
    };

    STDMETHODIMP CUpdateCallback::GetVolumeSize(UInt32 UNUSED(index), UInt64* /*size*/) throw() {
        DEBUGLOG(this << " CUpdateCallback::GetVolumeSize " << index);
        return S_FALSE;
    };

    STDMETHODIMP CUpdateCallback::GetVolumeStream(UInt32 UNUSED(index), ISequentialOutStream** /*volumeStream*/) throw() {
        DEBUGLOG(this << " CUpdateCallback::GetVolumeStream " << index);
        return S_FALSE;
    };

    STDMETHODIMP CUpdateCallback::CryptoGetTextPassword2(Int32* passwordIsDefined, BSTR* password) throw() {
        DEBUGLOG(this << " CUpdateCallback::CryptoGetTextPassword2 " << passworddefined);
        *password = NULL;
        *passwordIsDefined = BoolToInt(passworddefined);
        if (!passworddefined)
            return S_OK;
        return StringToBstr(this->password, password);
    };

    // archives

    Iarchive::Impl::Impl() {
        DEBUGLOG(this << " Iarchive::Impl::Impl");
    };

    Iarchive::Impl::~Impl() {
        DEBUGLOG(this << " Iarchive::Impl::~Impl");
        close();
    };

    HRESULT Iarchive::Impl::open(Lib::Impl* libimpl, Istream* istream,
            const wchar_t* filename, const wchar_t* password, int formatIndex) {
        DEBUGLOG(this << " Iarchive::open "
                << (filename ? filename : L"NULL") << " "
                << (password ? password : L"NULL") << " "
                << formatIndex);

        if (!libimpl || !libimpl->CreateObjectFunc)
            return S_FALSE;

        if (inarchive)
            return S_FALSE;

        HRESULT hr = S_OK;
        UString name = filename ? filename : L"";
        hr = istream->Open(name);
        if (FAILED(hr))
            return hr;
        // NOTE: commented to allow preopen stream start at nonzero positions (not checked yet)
        // hr = istream->Seek(0, SZ_SEEK_SET, nullptr);
        // if (FAILED(hr))
        //     return hr;

        instream = new CInStream(istream);
        opencallback = new COpenCallback(istream, name, password);

        const UInt64 scan = (UInt64)1 << 23;
        while (true) {

            UString ext = getFilenameExt(name.Ptr());

            // DEBUGLOG(this << " Iarchive::open name " << name.Ptr() << " formatIndex " << formatIndex);

            // search for signature for a given ext
            if (formatIndex == -1)
                formatIndex = libimpl->getFormatBySignature(instream, ext.Ptr());
            // signature not found, embedded archive? let 7z detect
            if (formatIndex == -1)
                formatIndex = libimpl->getFormatByExtension(ext.Ptr());
            // detect by ext is not requested or supported, check all signatures 
            if (formatIndex < 0)
                formatIndex = libimpl->getFormatBySignature(instream, nullptr);
            // unknown stream
            if (formatIndex < 0)
                return E_NOTSUPPORTED;

            this->formatIndex = formatIndex;

            DEBUGLOG(this << " Iarchive::open format " << formatIndex
                    << L" (" << libimpl->getFormatName(formatIndex) << L")");

            GUID guid = libimpl->getFormatGUID(formatIndex);

            // DEBUGLOG(this << " Iarchive::open CreateObjectFunc guid " << guid.Data1 << "-" << guid.Data2 << "-" << guid.Data3);
            hr = libimpl->CreateObjectFunc(&guid, &IID_IInArchive, (void**)&inarchive);
            if (hr != S_OK)
                return hr;

            // DEBUGLOG(this << " Iarchive::open inarchive->Open");
            hr = inarchive->Open(instream, &scan, opencallback);
            if (hr == S_FALSE)
                return E_NOTSUPPORTED;
            if (hr != S_OK)
                return hr;

            // DEBUGLOG(this << " Iarchive::open inarchive->GetNumberOfItems");
            UInt32 nitems = 0;
            hr = inarchive->GetNumberOfItems(&nitems);
            if (hr != S_OK)
                return hr;

            // DEBUGLOG(this << " Iarchive::open getIntProperty kpidMainSubfile");
            UInt32 mainsubfile = (UInt32)(Int32)- 1;
            hr = getIntProperty(kpidMainSubfile, mainsubfile);
            if (hr == S_FALSE)
                return S_OK; // no more subfiles
            if (hr != S_OK)
                return hr;

            // DEBUGLOG(this << " Iarchive::open getArchiveStringItemProperty kpidPath " << mainsubfile);
            hr = getArchiveStringItemProperty(inarchive, mainsubfile, kpidPath, name);
            if (hr != S_OK)
                return hr;

            // DEBUGLOG(this << " Iarchive::open QueryInterface IID_IInArchiveGetStream");
            CMyComPtr<IInArchiveGetStream> getStream = nullptr;
            hr = inarchive->QueryInterface(IID_IInArchiveGetStream, (void**)&getStream);
            if (hr != S_OK)
                return hr;
            if (!getStream)
                return S_FALSE;

            // DEBUGLOG(this << " Iarchive::open getStream->GetStream");
            CMyComPtr<ISequentialInStream> insubstream = nullptr;
            hr = getStream->GetStream(mainsubfile, &insubstream);
            if (hr != S_OK)
                return hr;
            if (!insubstream)
                return S_FALSE;

            // DEBUGLOG(this << " Iarchive::open insubstream->QueryInterface IID_IInStream");
            instream = nullptr;
            hr = insubstream.QueryInterface(IID_IInStream, &instream);
            if (hr != S_OK)
                return hr;
            if (!instream)
                return S_FALSE;

            // DEBUGLOG(this << " Iarchive::open QueryInterface IID_IArchiveOpenSetSubArchiveName");
            CMyComPtr<IArchiveOpenSetSubArchiveName> insetsubname;
            hr = opencallback->QueryInterface(IID_IArchiveOpenSetSubArchiveName, (void**)&insetsubname);
            if (hr != S_OK)
                return hr;
            if (!insetsubname)
                return S_FALSE;

            insetsubname->SetSubArchiveName(name);
            inarchives.Add(inarchive);
            inarchive = nullptr; // input stream leak w/o this assignment
            formatIndex = -1;
        }
    };

    void Iarchive::Impl::close() {
        DEBUGLOG(this << " Iarchive::close");
        for (int i = inarchives.Size(); i-- > 0; )
            inarchives[i] = nullptr;
        inarchives.Clear();
        inarchive = nullptr;
        instream = nullptr;
        opencallback = nullptr;
        formatIndex = -1;
    }

    HRESULT Iarchive::Impl::extract(Ostream* ostream, const wchar_t* password, int index) {
        if (!inarchive)
            return E_FAIL;

        CMyComPtr<IArchiveExtractCallback> extractcallback =
                new CExtractCallback(ostream, inarchive,
                password ? password : COPENCALLBACK(opencallback)->Password());

        DEBUGLOG(this << " Iarchive::Impl::extract index " << index);
        UInt32 items[1] = {(UInt32)(Int32)index};
        if (index < 0)
            return inarchive->Extract(nullptr, (UInt32)(Int32)(-1), false, extractcallback);
        else if (index < getNumberOfItems())
            return inarchive->Extract(items, 1, false, extractcallback);
        else
            return E_INVALIDARG;
    }

    int Iarchive::Impl::getNumberOfItems() {
        UInt32 n;
        if (inarchive && inarchive->GetNumberOfItems(&n) == S_OK)
            return n;
        return 0;
    };

    wchar_t* Iarchive::Impl::getItemPath(int index) {
        UString path;
		lastItemPath[0] = L'\0';
        if (!inarchive)
            return lastItemPath;
        if (getArchiveStringItemProperty(inarchive, index, kpidPath, path) == S_OK)
            COPYWCHARS(lastItemPath, path.Ptr());
        else
            COPYWCHARS(lastItemPath, kEmptyFileAlias);
        return lastItemPath;
    };

    UInt64 Iarchive::Impl::getItemSize(int index) {
        if (!inarchive)
            return 0;
        UInt64 size64;
        if (getArchiveWideItemProperty(inarchive, index, kpidSize, size64) == S_OK)
            return size64;
        UInt32 size32;
        if (getArchiveIntItemProperty(inarchive, index, kpidSize, size32) == S_OK)
            return size32;
        return 0;
    };

    UInt32 Iarchive::Impl::getItemMode(int index) {
        UInt32 mode;
        if (getArchiveIntItemProperty(inarchive, index, kpidPosixAttrib, mode) == S_OK)
            return mode;
        if (getArchiveIntItemProperty(inarchive, index, kpidAttrib, mode) == S_OK)
            if (mode & 0x8000)
                return mode >> 16; // p7zip posix feature
        return 0;
    };

    UInt32 Iarchive::Impl::getItemAttr(int index) {
        UInt32 attr;
        if (getArchiveIntItemProperty(inarchive, index, kpidAttrib, attr) == S_OK)
            return (attr & 0x8000) ? attr & 0x7FFF : attr;
        return 0;
    };

    UInt32 Iarchive::Impl::getItemTime(int index) {
        UInt32 time;
        if (getArchiveTimeItemProperty(inarchive, index, kpidMTime, time) == S_OK)
            return time;
        return 0;
    };

    bool Iarchive::Impl::getItemIsDir(int index) {
        bool isdir;
        if (!inarchive || getArchiveBoolItemProperty(inarchive, index, kpidIsDir, isdir) != S_OK)
            return false;
        return isdir;
    };

    int Iarchive::Impl::getNumberOfProperties() {
        UInt32 n;
        if (inarchive && inarchive->GetNumberOfArchiveProperties(&n) == S_OK)
            return n;
        return 0;
    };

    HRESULT Iarchive::Impl::getPropertyInfo(int propIndex, PROPID& propId, VARTYPE& propType) {
        CMyComBSTR name;
        if (!inarchive)
            return S_FALSE;
        HRESULT hr = inarchive->GetArchivePropertyInfo(propIndex, &name, &propId, &propType);
        if (hr != S_OK)
            return hr;
        return S_OK;
    };

    HRESULT Iarchive::Impl::getStringProperty(PROPID propId, const wchar_t*& propValue) {
        if (!inarchive)
            return E_FAIL;
        NWindows::NCOM::CPropVariant prop;
        HRESULT hr = inarchive->GetArchiveProperty(propId, &prop);
        if (hr != S_OK)
            return hr;
        if (prop.vt == VT_EMPTY)
            return S_FALSE;
        if (prop.vt == VT_BSTR)
            propValue = COPYWCHARS(lastStringProperty, prop.bstrVal);
        else
            return E_FAIL;
        return S_OK;
        // NOTE: alternative implementation
        // UString us;
        // if (getStringValue(prop, us) >= S_OK)
        //    propValue = COPYWCHARS(lastStringProperty, us.Ptr());
        // return hr;
    };

    HRESULT Iarchive::Impl::getBoolProperty(PROPID propId, bool& propValue) {
        if (!inarchive)
            return E_FAIL;
        NWindows::NCOM::CPropVariant prop;
        HRESULT hr = inarchive->GetArchiveProperty(propId, &prop);
        if (hr != S_OK)
            return hr;
        return getBoolValue(prop, propValue);
    };

    HRESULT Iarchive::Impl::getIntProperty(PROPID propId, UInt32& propValue) {
        if (!inarchive)
            return E_FAIL;
        NWindows::NCOM::CPropVariant prop;
        HRESULT hr = inarchive->GetArchiveProperty(propId, &prop);
        if (hr != S_OK)
            return hr;
        return getIntValue(prop, propValue);
    };

    HRESULT Iarchive::Impl::getWideProperty(PROPID propId, UInt64& propValue) {
        if (!inarchive)
            return E_FAIL;
        NWindows::NCOM::CPropVariant prop;
        HRESULT hr = inarchive->GetArchiveProperty(propId, &prop);
        if (hr != S_OK)
            return hr;
        return getWideValue(prop, propValue);
    };

    HRESULT Iarchive::Impl::getTimeProperty(PROPID propId, UInt32& propValue) {
        if (!inarchive)
            return E_FAIL;
        NWindows::NCOM::CPropVariant prop;
        HRESULT hr = inarchive->GetArchiveProperty(propId, &prop);
        if (hr != S_OK)
            return hr;
        return getTimeValue(prop, propValue);
    }

    int Iarchive::Impl::getNumberOfItemProperties() {
        UInt32 n;
        if (inarchive && inarchive->GetNumberOfProperties(&n) == S_OK)
            return n;
        return 0;
    };

    HRESULT Iarchive::Impl::getItemPropertyInfo(int propIndex, PROPID& propId, VARTYPE& propType) {
        CMyComBSTR name;
        if (!inarchive)
            return S_FALSE;
        HRESULT hr = inarchive->GetPropertyInfo(propIndex, &name, &propId, &propType);
        if (hr != S_OK)
            return hr;
        return S_OK;
    };

    HRESULT Iarchive::Impl::getStringItemProperty(int index, PROPID propId, const wchar_t*& propValue) {
        if (!inarchive)
            return E_FAIL;
        NWindows::NCOM::CPropVariant prop;
        HRESULT hr = inarchive->GetProperty(index, propId, &prop);
        if (hr != S_OK)
            return hr;
        if (prop.vt == VT_EMPTY)
            return S_FALSE;
        if (prop.vt == VT_BSTR)
            propValue = COPYWCHARS(lastStringProperty, prop.bstrVal);
        else
            return E_FAIL;
        return S_OK;
        // NOTE: alternative implementation
        // UString us;
        // HRESULT hr = getArchiveStringItemProperty(inarchive, index, propId, us);
        // if (hr >= S_OK)
        //    propValue = COPYWCHARS(lastStringProperty, us.Ptr());
        // return hr;
    };

    HRESULT Iarchive::Impl::getBoolItemProperty(int index, PROPID propId, bool& propValue) {
        if (!inarchive)
            return E_FAIL;
        return getArchiveBoolItemProperty(inarchive, index, propId, propValue);
    };

    HRESULT Iarchive::Impl::getIntItemProperty(int index, PROPID propId, UInt32& propValue) {
        if (!inarchive)
            return E_FAIL;
        return getArchiveIntItemProperty(inarchive, index, propId, propValue);
    };

    HRESULT Iarchive::Impl::getWideItemProperty(int index, PROPID propId, UInt64& propValue) {
        if (!inarchive)
            return E_FAIL;
        return getArchiveWideItemProperty(inarchive, index, propId, propValue);
    };

    HRESULT Iarchive::Impl::getTimeItemProperty(int index, PROPID propId, UInt32& propValue) {
        if (!inarchive)
            return E_FAIL;
        return getArchiveTimeItemProperty(inarchive, index, propId, propValue);
    };


    Oarchive::Impl::Impl() {
        DEBUGLOG(this << " Oarchive::Impl::Impl");
    };

    Oarchive::Impl::~Impl() {
        DEBUGLOG(this << " Oarchive::Impl::~Impl");
    };

    HRESULT Oarchive::Impl::open(Lib::Impl* libimpl,  Istream* istream, Ostream* ostream,
            const wchar_t* filename, const wchar_t* password, int formatIndex) {
        DEBUGLOG(this << " Oarchive::open " << istream << " " << ostream
                << " " << (filename ? filename : L"NULL")
                << " " << (password ? password : L"NULL")
                << " " << formatIndex);

        if (!libimpl || !libimpl->CreateObjectFunc)
            return S_FALSE;

        if (outarchive)
            return S_FALSE;

        HRESULT hr = ostream->Open(filename);
        if (FAILED(hr))
            return hr;

        close();

        if (formatIndex < 0)
            formatIndex = libimpl->getFormatByExtension(getFilenameExt(filename));
        if (formatIndex < 0)
            formatIndex = libimpl->getFormatByExtension(L"7z");
        if (formatIndex < 0)
            return E_NOTSUPPORTED;

        DEBUGLOG(this << " Oarchive::open format " << formatIndex
                << L" (" << libimpl->getFormatName(formatIndex) << L")");

        this->formatIndex = formatIndex;

        GUID guid = libimpl->getFormatGUID(formatIndex);

        updatecallback = new CUpdateCallback(istream, password);
        outstream = new COutStream(ostream);
        return libimpl->CreateObjectFunc(&guid, &IID_IOutArchive, (void**)&outarchive);
    };
    
    void Oarchive::Impl::close() {
        DEBUGLOG(this << " Oarchive::close");
        outarchive = nullptr;
        outstream = nullptr;
        updatecallback = nullptr;
        formatIndex = -1;
    };

    void Oarchive::Impl::addItem(const wchar_t* pathname) {
        DEBUGLOG(this << " Oarchive::addItem " << pathname);
        if (updatecallback)
            CUPDATECALLBACK(updatecallback)->items.Add(pathname);
    };

    HRESULT Oarchive::Impl::update() {
        DEBUGLOG(this << " Oarchive::update");

        if (!outstream)
            return S_FALSE;
        if (!outarchive)
            return S_FALSE;
        if (!updatecallback)
            return S_FALSE;

        HRESULT hr = outarchive->UpdateItems(outstream,
            CUPDATECALLBACK(updatecallback)->items.Size(), updatecallback);
        if (hr == S_OK)
            CUPDATECALLBACK(updatecallback)->items.Clear();
        return hr;
    };

    HRESULT Oarchive::Impl::setEmptyProperty(const wchar_t* name) {
        DEBUGLOG(this << " Oarchive::setEmptyProperty " << name);

        if (!outarchive || !name)
            return S_FALSE;

        NWindows::NCOM::CPropVariant prop;
        return setProperty(outarchive, name, prop);
    };

    HRESULT Oarchive::Impl::setStringProperty(const wchar_t* name, const wchar_t* value) {
        DEBUGLOG(this << " Oarchive::setStringProperty " << name << " " << (value ? value : L"NULL"));

        if (!outarchive || !name)
            return S_FALSE;

        NWindows::NCOM::CPropVariant prop = L"";
        if (value)            
            prop = value;
        return setProperty(outarchive, name, prop);
    };

    HRESULT Oarchive::Impl::setBoolProperty(const wchar_t* name, bool value) {
        DEBUGLOG(this << " Oarchive::setBoolProperty " << name << " " << value);

        if (!outarchive || !name)
            return S_FALSE;

        NWindows::NCOM::CPropVariant prop = value;
        return setProperty(outarchive, name, prop);
    };

    HRESULT Oarchive::Impl::setIntProperty(const wchar_t* name, UInt32 value) {
        DEBUGLOG(this << " Oarchive::setIntProperty " << name << " " << value);

        if (!outarchive || !name)
            return S_FALSE;

        NWindows::NCOM::CPropVariant prop = value;
        return setProperty(outarchive, name, prop);
    };

    HRESULT Oarchive::Impl::setWideProperty(const wchar_t* name, UInt64 value) {
        DEBUGLOG(this << " Oarchive::setWideProperty " << name << " " << value);

        if (!outarchive || !name)
            return S_FALSE;

        NWindows::NCOM::CPropVariant prop = value;
        return setProperty(outarchive, name, prop);
    };

    // library

    Lib::Impl::Impl() {
        DEBUGLOG(this << " Lib::Impl::Impl");
    };

    Lib::Impl::~Impl() {
        DEBUGLOG(this << " Lib::Impl::~Impl");
		unload();
    };

    void Lib::Impl::unload() {
        DEBUGLOG(this << " Lib::Impl::unload" << lib);
        if (lib) {
            // NOTE: library handle must be preserved to avoid dependent modules crashes
            // NOTE: we need to count references to the library handle if we want to unload it safely
#ifdef _WIN32
            // ::FreeLibrary((HMODULE)lib);
#else
            // dlclose(lib);
#endif
            lib = nullptr;
        }
    };

    // NOTE: used internally instead of incomplete unload
    void Lib::Impl::_unload() {
        DEBUGLOG(this << " Lib::Impl::_unload " << lib);
        if (lib) {
#ifdef _WIN32
            ::FreeLibrary((HMODULE)lib);
#else
            dlclose(lib);
#endif
            lib = nullptr;
        }
    }


    bool Lib::Impl::load(const wchar_t* libname) {
        DEBUGLOG(this << " Lib::Impl::Load " << (libname ? libname : L"NULL"));
        loadMessage[0] = '\0';
        if (lib)
            return true;
        if (!libname)
            return false;
        do {
#ifdef _WIN32
            lib = ::LoadLibraryW(libname);
#else
            lib = dlopen(us2as(libname), RTLD_NOW);
#endif
            if (!lib)
                break;
            GetModuleProp = (Func_GetModuleProp)GetProcAddress("GetModuleProp");
            if (!checkInterfaceType()) {
                COPYACHARS(loadMessage, "Library interface type mismatch");
                _unload();
                return false;
            }
            CreateObjectFunc = (Func_CreateObject)GetProcAddress("CreateObject");
            if (!CreateObjectFunc)
                break;
            GetNumberOfMethods = (Func_GetNumberOfMethods)GetProcAddress("GetNumberOfMethods");
            if (!GetNumberOfMethods)
                break;
            GetNumberOfFormats = (Func_GetNumberOfFormats)GetProcAddress("GetNumberOfFormats");
            if (!GetNumberOfFormats)
                break;
            GetMethodProperty = (Func_GetMethodProperty)GetProcAddress("GetMethodProperty");
            if (!GetMethodProperty)
                break;
            GetHandlerProperty = (Func_GetHandlerProperty)GetProcAddress("GetHandlerProperty");
            if (!GetHandlerProperty)
                break;
            GetHandlerProperty2 = (Func_GetHandlerProperty2)GetProcAddress("GetHandlerProperty2");
            if (!GetHandlerProperty2)
                break;
            DEBUGLOG(this << " Lib::Impl::Load success : " << lib);
            return true;
        } while (0);
#ifdef _WIN32
        COPYWCHARS(loadMessage, NWindows::NError::MyFormatMessage(GetLastError()).Ptr());
#else
        COPYACHARS(loadMessage, dlerror());
#endif
        GetModuleProp = nullptr;
        CreateObjectFunc = nullptr;
        GetNumberOfMethods = nullptr;
        GetNumberOfFormats = nullptr;
        GetHandlerProperty = nullptr;
        GetMethodProperty = nullptr;
        GetHandlerProperty2 = nullptr;
        DEBUGLOG(this << " Lib::Impl::Load error : " << loadMessage);
        _unload();
        return false;
    };

    bool Lib::Impl::isLoaded() const {
		return lib != nullptr;
    };

    wchar_t* Lib::Impl::getLoadMessage() {
        return loadMessage;
    };

    unsigned int Lib::Impl::getVersion() {
        NWindows::NCOM::CPropVariant prop;
        if (!GetModuleProp)
            return 0;
        if (GetModuleProp(NModulePropID::kVersion, &prop) != S_OK)
            return 0;
        if (prop.vt != VT_UI4)
            return 0;
        return prop.ulVal;
    };

    int Lib::Impl::getNumberOfMethods() {
        UInt32 n = 1;
        if (!GetNumberOfMethods)
            return 0;
        if (GetNumberOfMethods(&n) != S_OK)
            return 0;
        return n;
    };

    wchar_t* Lib::Impl::getMethodName(int index) {
        lastMethodName[0] = L'\0';
        NWindows::NCOM::CPropVariant prop;
        if (!GetMethodProperty)
            return lastMethodName;
        if (GetMethodProperty(index, NMethodPropID::kName, &prop) != S_OK)
            return lastMethodName;
        if (prop.vt != VT_BSTR)
            return lastMethodName;
        COPYWCHARS(lastMethodName, prop.bstrVal);
        return lastMethodName;
    };

    // NOTE: usable props - kDecoderIsAssigned, kEncoderIsAssigned, kIsFilter
    // bool Lib::Impl::getMethodIsEncoder(int index) {
    //     NWindows::NCOM::CPropVariant prop;
    //     if (!GetMethodProperty)
    //         return false;
    //     if (GetMethodProperty(index, NMethodPropID::kEncoderIsAssigned, &prop) != S_OK)
    //         return false;
    //     if (prop.vt != VT_BOOL)
    //         return false;
    //     return prop.boolVal;
    // };

    int Lib::Impl::getNumberOfFormats() {
        UInt32 n = 1;
        if (!GetNumberOfFormats)
            return 0;
        if (GetNumberOfFormats(&n) != S_OK)
            return 0;
        return n;
    };

    wchar_t* Lib::Impl::getFormatExtensions(int index) {
        lastFormatExtensions[0] = L'\0';
        NWindows::NCOM::CPropVariant prop;
        if (!GetHandlerProperty2)
            return lastFormatExtensions;
        if (GetHandlerProperty2(index, NArchive::NHandlerPropID::kExtension, &prop) != S_OK)
            return lastFormatExtensions;
        if (prop.vt != VT_BSTR)
            return lastFormatExtensions;
        COPYWCHARS(lastFormatExtensions, prop.bstrVal);
        return lastFormatExtensions;
    };

    wchar_t* Lib::Impl::getFormatName(int index) {
        lastFormatName[0] = L'\0';
        NWindows::NCOM::CPropVariant prop;
        if (!GetHandlerProperty2)
            return lastFormatName;
        if (GetHandlerProperty2(index, NArchive::NHandlerPropID::kName, &prop) != S_OK)
            return lastFormatName;
        if (prop.vt != VT_BSTR)
            return lastFormatName;
        COPYWCHARS(lastFormatName, prop.bstrVal);
        return lastFormatName;
    };

    bool Lib::Impl::getFormatUpdatable(int index) {
        NWindows::NCOM::CPropVariant prop;
        if (!GetHandlerProperty2)
            return false;
        if (GetHandlerProperty2(index, NArchive::NHandlerPropID::kUpdate, &prop) != S_OK)
            return false;
        if (prop.vt != VT_BOOL)
            return false;
        return prop.boolVal;
    };

    int Lib::Impl::getFormatByExtension(const wchar_t* ext) {
        if (!ext)
            return -1;
        for (int i = 0; i < getNumberOfFormats(); i++) {
            if (isExtensionSupported(i, ext))
                return i;
        }
        return -1;
    };

    int Lib::Impl::getFormatBySignature(Istream* stream, const wchar_t* ext) {
        CInStream instream(stream);
        return getFormatBySignature(&instream, ext);
    };

    int Lib::Impl::getFormatBySignature(IInStream* stream, const wchar_t* ext) {
        if (!GetHandlerProperty2)
            return -1;
        UInt64 pos = 0, end;
        UInt32 bufsize = 2048;
        CByteBuffer buf(bufsize);
        buf.Wipe();
        if (stream->Seek(0, SZ_SEEK_CUR, &pos) != S_OK)
            return -1;
        if (stream->Seek(0, SZ_SEEK_END, &end) != S_OK)
            return -1;
        if (stream->Seek(0, SZ_SEEK_SET, nullptr) != S_OK)
            return -1;
        if (stream->Read(buf, bufsize, nullptr) != S_OK)
            return -1;
        if (stream->Seek(pos, SZ_SEEK_SET, nullptr) != S_OK)
            return -1;
        CByteBuffer buf2; // dynamic buffer

        for (int i = 0; i < getNumberOfFormats(); i++) {
            // DEBUGLOG(this << " getFormatBySignature checking format " << i << " "
            //     << getFormatName(i) << " ext " << (ext ? ext : L"NULL"));

            // restrict detection to a given extension if is not empty
            if (ext && ext[0] && !isExtensionSupported(i, ext))
                continue;

            NWindows::NCOM::CPropVariant prop, prop1, prop2;
            UINT len1 = 0, len2 = 0;
            ULONG offs = 0;

            if (GetHandlerProperty2(i, NArchive::NHandlerPropID::kSignatureOffset, &prop) == S_OK)
                if (prop.vt == VT_UI4)
                    offs = prop.ulVal;
            if (GetHandlerProperty2(i, NArchive::NHandlerPropID::kSignature, &prop1) == S_OK)
                if (prop1.vt == VT_BSTR)
                    len1 = SysStringByteLen(prop1.bstrVal);
            if (GetHandlerProperty2(i, NArchive::NHandlerPropID::kMultiSignature, &prop2) == S_OK)
                if (prop2.vt == VT_BSTR)
                    len2 = SysStringByteLen(prop2.bstrVal);

            DEBUGLOG(this << " getFormatBySignature " << i << " " << offs << "/" << len1 << "/" <<  len2);
            
            // signature not defined, return the first format that matches the extension
            if (ext && ext[0] && len1 == 0 && len2 == 0)
                return i;

            CByteBuffer *bufptr = &buf;
            bool isdmg = getFormatGUID(i).Data4[5] == 0xE4;

            // process dmg or other format with signature after first 2048 bytes (iso, udf)
            if (offs + max(len1, len2) > bufsize || isdmg) {

                DEBUGLOG(this << " getFormatBySignature " << i << " using dynamic buffer, isdmg " << isdmg);

                if (isdmg) {
                    if (end < 512)
                        continue;
                    if (stream->Seek(-512, SZ_SEEK_END, nullptr) != S_OK)
                        return -1;
                } else {
                    if (end < offs + max(len1, len2))
                        continue;
                    if (stream->Seek(offs, SZ_SEEK_SET, nullptr) != S_OK)
                        return -1;
                }
                buf2.AllocAtLeast(max(max(len1, len2), 64));
                buf2.Wipe();
                if (stream->Read(buf2, max(len1, len2), nullptr) != S_OK)
                    return -1;
                if (stream->Seek(pos, SZ_SEEK_SET, nullptr) != S_OK)
                    return -1;
                bufptr = &buf2;
                offs = 0;
            }
            if (len1 > 0) {
                if (memcmp(prop1.bstrVal, *bufptr + offs, len1) == 0)
                    return i;
            }
            if (len2 > 0) {
                auto sign = reinterpret_cast<const Byte*>(prop2.bstrVal);
                auto rest = len2;
                while (rest > 0) {
                    const unsigned len = *sign++;
                    rest--;
                    if (len > rest)
                        break;
                    if (len > 0 && memcmp(sign, *bufptr + offs, len) == 0)
                        return i;
                    sign += len;
                    rest -= len;
                }
            }

            // DEBUGLOG(this << " getFormatBySignature " << i << " not detected ");
        }
        return -1;
    };

    GUID Lib::Impl::getFormatGUID(int index) {
        NWindows::NCOM::CPropVariant prop;
        if (!GetHandlerProperty2)
            return IID_IUnknown;
        if (GetHandlerProperty2(index, NArchive::NHandlerPropID::kClassID, &prop) != S_OK)
            return IID_IUnknown;
        if (prop.vt != VT_BSTR)
            return IID_IUnknown;
        if (SysStringByteLen(prop.bstrVal) != sizeof(GUID))
            return IID_IUnknown;
        return *(const GUID*)(const void*)prop.bstrVal;
    };

    bool Lib::Impl::isExtensionSupported(int index, const wchar_t* ext) {    
        wchar_t* exts = getFormatExtensions(index);
        wchar_t* state;
        wchar_t* token = wcstok(exts, L" ", &state);
        while (token) {
            if (wcscmp(token, ext) == 0)
                return true;
            token = wcstok(NULL, L" ", &state);
        }
        return false;
    };

    UString Lib::Impl::getStringProperty(int propIndex, PROPID propID) {
        NWindows::NCOM::CPropVariant prop;
        if (!GetHandlerProperty2)
            return L"";
        if (GetHandlerProperty2(propIndex, propID, &prop) != S_OK)
            return L"";
        if (prop.vt != VT_BSTR)
            return L"";
        return (UString)prop.bstrVal;
    };

    bool Lib::Impl::checkInterfaceType() const {
        UInt32 flags =
#ifdef _WIN32
            NModuleInterfaceType::k_IUnknown_VirtDestructor_No;
#else
            NModuleInterfaceType::k_IUnknown_VirtDestructor_Yes;
#endif
        if (GetModuleProp) {
            NWindows::NCOM::CPropVariant prop;
            if (GetModuleProp(NModulePropID::kInterfaceType, &prop) == S_OK)
            {
                if (prop.vt == VT_UI4)
                    flags = prop.ulVal;
            }
        }
        DEBUGLOG(this << " interface flags "
            << NModuleInterfaceType::k_IUnknown_VirtDestructor_ThisModule
            << " vs " << flags);
        return flags == NModuleInterfaceType::k_IUnknown_VirtDestructor_ThisModule;
    };

    void* Lib::Impl::GetProcAddress(const char* proc) {
        if (!lib)
            return nullptr;
#ifdef _WIN32
        return (void*)::GetProcAddress(lib, proc);
#else
        return dlsym(lib, proc);
#endif
    }
}
