#include <iostream>
#include <cwchar>
#include "sevenzip.h"

static void CHECK(bool cond, const char* msg) {
    if (!cond) {
        std::cerr << "FAIL: " << msg << std::endl;
        throw std::runtime_error(msg);
    }
}

// A minimal fake Istream implementation
struct FakeIstream : public sevenzip::Istream {
    bool open_ok = true;
    bool seek_ok = true;
    bool read_ok = true;
    virtual HRESULT Open(const wchar_t* /*filename*/) override {
        return open_ok ? S_OK : S_FALSE;
    }
    virtual HRESULT Read(void* /*data*/, UInt32 /*size*/, UInt32& processed) override {
		processed = 0;
        return read_ok ? S_OK : S_FALSE;
    }
    virtual void Close() override {}
    virtual HRESULT Seek(Int64 /*offset*/, UInt32 /*origin*/, UInt64& /*position*/) override {
        return seek_ok ? S_OK : S_FALSE;
    }
};

void run_lib_tests() {
    std::cout << "Running lib tests... ";
    sevenzip::Lib l;
    FakeIstream in;

    // Loading a non-existing library should fail and provide a message
    bool ok = l.load(L"no_such_library");
    CHECK(!ok, "Lib::load should return false for missing library");
    const wchar_t* msg = l.getLoadMessage();
    CHECK(msg && wcslen(msg) > 0, "Lib::getLoadMessage should return a non-empty string when load failed");

    // When library not loaded, these queries should return empty/zero values
    CHECK(l.getVersion() == 0, "Lib::getVersion should be 0 when library not loaded");
    CHECK(l.getNumberOfFormats() == 0, "Lib::getNumberOfFormats should be 0 when library not loaded");
    CHECK(l.getFormatByExtension(L"7z") == -1, "Lib::getFormatByExtension should return -1 when no formats available");
    CHECK(l.getFormatBySignature(in) == -1, "Lib::getFormatBySignature should return -1 when no formats available");

    std::cout << "lib tests passed." << std::endl;
}
