#include <fstream>
#include <iostream>
#include "sevenzip.h"

#include <sys/stat.h>

#ifdef _WIN32
#define U2F(_s_) (_s_)
#define LOCALTIME(_t_,_i_) localtime_s((_i_),(_t_))
#else
#include <locale>
#include <codecvt>
std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
#define U2F(_s_) (convert.to_bytes(_s_).c_str())
#define LOCALTIME(_t_,_i_) localtime_r((_t_),(_i_))
#endif

using namespace std;
using namespace sevenzip;

static const wchar_t * const propNames[] = {
    L"0"
  , L"1"
  , L"2"
  , L"Path"
  , L"Name"
  , L"Extension"
  , L"Folder"
  , L"Size"
  , L"Packed Size"
  , L"Attributes"
  , L"Created"
  , L"Accessed"
  , L"Modified"
  , L"Solid"
  , L"Commented"
  , L"Encrypted"
  , L"Split Before"
  , L"Split After"
  , L"Dictionary Size"
  , L"CRC"
  , L"Type"
  , L"Anti"
  , L"Method"
  , L"Host OS"
  , L"File System"
  , L"User"
  , L"Group"
  , L"Block"
  , L"Comment"
  , L"Position"
  , L"Path Prefix"
  , L"Folders"
  , L"Files"
  , L"Version"
  , L"Volume"
  , L"Multivolume"
  , L"Offset"
  , L"Links"
  , L"Blocks"
  , L"Volumes"
  , L"Time Type"
  , L"64-bit"
  , L"Big-endian"
  , L"CPU"
  , L"Physical Size"
  , L"Headers Size"
  , L"Checksum"
  , L"Characteristics"
  , L"Virtual Address"
  , L"ID"
  , L"Short Name"
  , L"Creator Application"
  , L"Sector Size"
  , L"Mode"
  , L"Symbolic Link"
  , L"Error"
  , L"Total Size"
  , L"Free Space"
  , L"Cluster Size"
  , L"Label"
  , L"Local Name"
  , L"Provider"
  , L"NT Security"
  , L"Alternate Stream"
  , L"Aux"
  , L"Deleted"
  , L"Tree"
  , L"SHA-1"
  , L"SHA-256"
  , L"Error Type"
  , L"Errors"
  , L"Errors"
  , L"Warnings"
  , L"Warning"
  , L"Streams"
  , L"Alternate Streams"
  , L"Alternate Streams Size"
  , L"Virtual Size"
  , L"Unpack Size"
  , L"Total Physical Size"
  , L"Volume Index"
  , L"SubType"
  , L"Short Comment"
  , L"Code Page"
  , L"Is not archive type"
  , L"Physical Size can't be detected"
  , L"Zeros Tail Is Allowed"
  , L"Tail Size"
  , L"Embedded Stub Size"
  , L"Link"
  , L"Hard Link"
  , L"iNode"
  , L"Stream ID"
  , L"Read-only"
  , L"Out Name"
  , L"Copy Link"
  , L"ArcFileName"
  , L"IsHash"
  , L"Metadata Changed"
  , L"User ID"
  , L"Group ID"
  , L"Device Major"
  , L"Device Minor"
  , L"Dev Major"
  , L"Dev Minor"
};

static wchar_t* ftime(UInt32 time) {
    static wchar_t buffer[64];
    time_t t = time;
    struct tm tm_info;
    LOCALTIME(&t, &tm_info);
    wcsftime(buffer, sizeof(buffer)/sizeof(buffer[0]), L"%Y-%m-%d %H:%M:%S", &tm_info);
    return buffer;
}

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
    wcout << "Show archive props\n\n";

    Lib l;
    if (!l.load(SEVENZIPDLL)) {
        wcerr << l.getLoadMessage() << "\n";
        return 1;
    }

    Iarchive a(l);
    Inputstream s;
    HRESULT hr = a.open(s, L"temps/example9.7z");
    wcout << "open : " << getMessage(hr) << "\n";
    wcout << "props :\n";
    int m =  a.getNumberOfProperties();
    for (int i = 0; i < m; i++) {
        PROPID propId;
        VARTYPE propType;
        wcout << "\t" << i+1 << " : ";
        if (a.getPropertyInfo(i, propId, propType) == S_OK) {
            const wchar_t* stringValue;
            bool boolValue;
            UInt32 uint32Value;
            UInt64 uint64Value;
            if (propId < sizeof(propNames)/sizeof(propNames[0]))
                wcout << propNames[propId] << "(" << propId << "/" << propType << ") = ";
            else 
                wcout << "unknown (" << propId << "/" << propType << ") = ";
            if (a.getStringProperty(propId, stringValue) == S_OK)
                wcout << stringValue << "\n";
            else if (a.getBoolProperty(propId, boolValue) == S_OK)
                wcout << boolValue << "\n";
            else if (a.getIntProperty(propId, uint32Value) == S_OK)
                wcout << uint32Value << "\n";
            else if (a.getWideProperty(propId, uint64Value) == S_OK)
                wcout << propId << " = " << uint64Value << "\n";
            else
                wcout << propId << " = " << "unknown" << propType << "\n";
       }
    }
    wcout << "items :\n";
    int n = a.getNumberOfItems();
    int k = a.getNumberOfItemProperties();
    for (int i = 0; i < n; i++) {
        wcout << "\t" << i+1 << " : " << a.getItemPath(i) << "\t";
        wcout << ftime(a.getItemTime(i)) << "\t";
        wcout << oct << a.getItemMode(i) << "\t";
        if (a.getItemIsDir(i))
            wcout << "DIR\n";
        else
            wcout << a.getItemSize(i) << " bytes\n";
        for (int j = 0; j < k; j++) {
            PROPID propId;
            VARTYPE propType;
            wcout << "\t\t" << j+1 << " : ";
            if (a.getItemPropertyInfo(j, propId, propType) == S_OK) {
                const wchar_t* stringValue;
                bool boolValue;
                UInt32 uint32Value;
                UInt64 uint64Value;
                if (propId < sizeof(propNames) / sizeof(propNames[0]))
                    wcout << propNames[propId] << "(" << propId << "/" << propType << ") = ";
                else
                    wcout << "unknown (" << propId << "/" << propType << ") = ";
                if (a.getStringItemProperty(i, propId, stringValue) == S_OK)
                    wcout << stringValue << "\n";
                else if (a.getBoolItemProperty(i, propId, boolValue) == S_OK)
                    wcout << boolValue << "\n";
                else if (a.getIntItemProperty(i, propId, uint32Value) == S_OK)
                    wcout << uint32Value << "\n";
                else if (a.getWideItemProperty(i, propId, uint64Value) == S_OK)
                    wcout << uint64Value << "\n";
                else
                    wcout << "unknown" << "\n";
            }
        }
    }

    a.close();
    
    if (n > 0 && m > 0 && k > 0) {
        wcout << "TEST PASSED\n";
    }
    return 0;
}
