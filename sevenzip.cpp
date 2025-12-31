#include "sevenzip_impl.h"

#ifdef _WIN32
extern
HINSTANCE g_hInstance;
HINSTANCE g_hInstance = 0;
#endif

namespace sevenzip {

    Lib::Lib() : pimpl(new Impl) {}

    Lib::~Lib() { delete pimpl; }

    bool Lib::load(const wchar_t* libname) {
        return pimpl->load(libname);
    }

    bool Lib::isLoaded() {
        return pimpl->isLoaded();
    }

    wchar_t* Lib::getLoadMessage() {
        return pimpl->getLoadMessage();
    }

    unsigned Lib::getVersion() {
        return pimpl->getVersion();
    }

    int Lib::getNumberOfFormats() {
        return pimpl->getNumberOfFormats();
    }

    int Lib::getFormatByExtension(const wchar_t* ext) {
        return pimpl->getFormatByExtension(ext);
    }

    int Lib::getFormatBySignature(Istream& stream) {
        return pimpl->getFormatBySignature(&stream);
    }

    wchar_t* Lib::getFormatName(int index) {
        return pimpl->getFormatName(index);
    }

    wchar_t* Lib::getFormatExtensions(int index) {
        return pimpl->getFormatExtensions(index);
    }

    bool Lib::getFormatUpdatable(int index) {
        return pimpl->getFormatUpdatable(index);
    }

    Iarchive::Iarchive(Lib& lib) : pimpl(new Impl(lib)) {};

    Iarchive::~Iarchive() { delete pimpl; };

    HRESULT Iarchive::open(Istream& istream,
            const wchar_t* path, int formatIndex) {
        return pimpl->open(&istream, path, nullptr, formatIndex);
    };

    HRESULT Iarchive::open(Istream& istream,
           const wchar_t* path, const wchar_t* password, int formatIndex) {
        return pimpl->open(&istream, path, password, formatIndex);
    };

    HRESULT Iarchive::extract(Ostream& ostream, int itemIndex) {
        return pimpl->extract(&ostream, nullptr, itemIndex);
    }

    HRESULT Iarchive::extract(Ostream& ostream, const wchar_t* password, int itemIndex) {
        return pimpl->extract(&ostream, password, itemIndex);
    }

    void Iarchive::close() {
        return pimpl->close();
    };

    int Iarchive::getNumberOfItems() {
        return pimpl->getNumberOfItems();
    };

    const wchar_t* Iarchive::getItemPath(int index) {
        return pimpl->getItemPath(index);
    };

    UInt64 Iarchive::getItemSize(int index) {
        return pimpl->getItemSize(index);
    };

    UInt32 Iarchive::getItemMode(int index) {
        return pimpl->getItemMode(index);
    };

    UInt32 Iarchive::getItemTime(int index) {
        return pimpl->getItemTime(index);
    };

    bool Iarchive::getItemIsDir(int index) {
        return pimpl->getItemIsDir(index);
    };

    int Iarchive::getNumberOfProperties() {
        return pimpl->getNumberOfProperties();
    };

    HRESULT Iarchive::getPropertyInfo(int propIndex, PROPID& propId, VARTYPE& propType) {
        return pimpl->getPropertyInfo(propIndex, propId, propType);
    };

    HRESULT Iarchive::getStringProperty(PROPID propId, const wchar_t*& propValue) {
        return pimpl->getStringProperty(propId, propValue);
    };

    HRESULT Iarchive::getBoolProperty(PROPID propId, bool& propValue) {
        return pimpl->getBoolProperty(propId, propValue);
    };

    HRESULT Iarchive::getIntProperty(PROPID propId, UInt32& propValue) {
        return pimpl->getIntProperty(propId, propValue);
    };
    
    HRESULT Iarchive::getWideProperty(PROPID propId, UInt64& propValue) {
        return pimpl->getWideProperty(propId, propValue);
    };

    HRESULT Iarchive::getTimeProperty(PROPID propId, UInt32& propValue) {
        return pimpl->getTimeProperty(propId, propValue);
    };

    int Iarchive::getNumberOfItemProperties() {
        return pimpl->getNumberOfItemProperties();
    };

    HRESULT Iarchive::getItemPropertyInfo(int propIndex, PROPID& propId, VARTYPE& propType) {
        return pimpl->getItemPropertyInfo(propIndex, propId, propType);
    }

    HRESULT Iarchive::getStringItemProperty(int index, PROPID propId, const wchar_t*& propValue) {
        return pimpl->getStringItemProperty(index, propId, propValue);
    };

    HRESULT Iarchive::getBoolItemProperty(int index, PROPID propId, bool& propValue) {
        return pimpl->getBoolItemProperty(index, propId, propValue);
    };

    HRESULT Iarchive::getIntItemProperty(int index, PROPID propId, UInt32& propValue) {
        return pimpl->getIntItemProperty(index, propId, propValue);
    };

    HRESULT Iarchive::getWideItemProperty(int index, PROPID propId, UInt64& propValue) {
        return pimpl->getWideItemProperty(index, propId, propValue);
    };

    HRESULT Iarchive::getTimeItemProperty(int index, PROPID propId, UInt32& propValue) {
        return pimpl->getTimeItemProperty(index, propId, propValue);
    };

    Oarchive::Oarchive(Lib& lib): pimpl(new Impl(lib)) {};

    Oarchive::~Oarchive() {delete pimpl;};

    HRESULT Oarchive::open(Istream& istream, Ostream& ostream,
            const wchar_t* filename, int formatIndex) {
        return pimpl->open(&istream, &ostream, filename, nullptr, formatIndex);
    };

    HRESULT Oarchive::open(Istream& istream, Ostream& ostream,
            const wchar_t* filename, const wchar_t* password, int formatIndex) {
        return pimpl->open(&istream, &ostream, filename, password, formatIndex);
    };

    HRESULT Oarchive::update() {
        return pimpl->update();
    };

    void Oarchive::addItem(const wchar_t* pathname) {
        pimpl->addItem(pathname);
    };

    void Oarchive::close() {
        pimpl->close();
    };
}
