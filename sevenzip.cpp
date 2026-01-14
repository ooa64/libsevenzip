#include "sevenzip_impl.h"

#ifdef _WIN32
extern
HINSTANCE g_hInstance;
HINSTANCE g_hInstance = 0;
#endif

namespace sevenzip {

    Lib::Lib() : pimpl(new Impl) {}

    Lib::~Lib() { delete pimpl; }

    void Lib::unload() {
        pimpl->unload();
    }

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

    int Lib::getNumberOfMethods() {
        return pimpl->getNumberOfMethods();
    }

    wchar_t* Lib::getMethodName(int index) {
        return pimpl->getMethodName(index);
    }

    int Lib::getNumberOfFormats() {
        return pimpl->getNumberOfFormats();
    }

    int Lib::getFormatByExtension(const wchar_t* ext) {
        return pimpl->getFormatByExtension(ext);
    }

    int Lib::getFormatBySignature(Istream& stream, const wchar_t* ext) {
        return pimpl->getFormatBySignature(&stream, ext);
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

    Iarchive::Iarchive() : pimpl(new Impl()) {};

    Iarchive::~Iarchive() { delete pimpl; };

    HRESULT Iarchive::open(Lib& lib, Istream& istream,
            const wchar_t* path, int formatIndex) {
        return pimpl->open(lib.pimpl, &istream, path, nullptr, formatIndex);
    };

    HRESULT Iarchive::open(Lib& lib, Istream& istream,
           const wchar_t* path, const wchar_t* password, int formatIndex) {
        return pimpl->open(lib.pimpl, &istream, path, password, formatIndex);
    };

    void Iarchive::close() {
        return pimpl->close();
    };

    HRESULT Iarchive::extract(Ostream& ostream, int itemIndex) {
        return pimpl->extract(&ostream, nullptr, itemIndex);
    };

    HRESULT Iarchive::extract(Ostream& ostream, const wchar_t* password, int itemIndex) {
        return pimpl->extract(&ostream, password, itemIndex);
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

    UInt32 Iarchive::getItemAttr(int index) {
        return pimpl->getItemAttr(index);
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
    };

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

    Oarchive::Oarchive(): pimpl(new Impl()) {};

    Oarchive::~Oarchive() {delete pimpl;};
    
    HRESULT Oarchive::open(Lib& lib, Istream& istream, Ostream& ostream,
            const wchar_t* filename, int formatIndex) {
        return pimpl->open(lib.pimpl, &istream, &ostream, filename, nullptr, formatIndex);
    };

    HRESULT Oarchive::open(Lib& lib, Istream& istream, Ostream& ostream,
            const wchar_t* filename, const wchar_t* password, int formatIndex) {
        return pimpl->open(lib.pimpl, &istream, &ostream, filename, password, formatIndex);
    };

    void Oarchive::close() {
        pimpl->close();
    };

    void Oarchive::addItem(const wchar_t* pathname) {
        pimpl->addItem(pathname);
    };

    HRESULT Oarchive::update() {
        return pimpl->update();
    };
    
    HRESULT Oarchive::setStringProperty(const wchar_t* name, const wchar_t* value) {
        return pimpl->setStringProperty(name, value);
    };

    HRESULT Oarchive::setBoolProperty(const wchar_t* name, bool value) {
        return pimpl->setBoolProperty(name, value);
    };

    HRESULT Oarchive::setIntProperty(const wchar_t* name, UInt32 value) {
        return pimpl->setIntProperty(name, value);
    };

    HRESULT Oarchive::setWideProperty(const wchar_t* name, UInt64 value) {
        return pimpl->setWideProperty(name, value);
    };

    HRESULT Oarchive::setEmptyProperty(const wchar_t* name) {
        return pimpl->setEmptyProperty(name);
    };
}
