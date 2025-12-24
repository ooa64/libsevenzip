#include <iostream>
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

    virtual HRESULT Open(const wchar_t* /*filename*/) override {
        return open_ok ? S_OK : S_FALSE;
    }
    virtual HRESULT Read(void* /*data*/, UInt32 /*size*/, UInt32* processed) override {
        if (processed) *processed = 0;
        return S_OK;
    }
    virtual void Close() override {}
    virtual HRESULT Seek(Int64 /*offset*/, UInt32 /*origin*/, UInt64* /*position*/) override {
        return seek_ok ? S_OK : S_FALSE;
    }
};

// Minimal fake Ostream
struct FakeOstream : public sevenzip::Ostream {
    bool open_ok = true;
    virtual HRESULT Open(const wchar_t* /*filename*/) override {
        return open_ok ? S_OK : S_FALSE;
    }
    virtual HRESULT Write(const void* /*data*/, UInt32 /*size*/, UInt32* processed) override {
        if (processed) *processed = 0;
        return S_OK;
    }
    virtual void Close() override {}
};

void run_iarchive_tests() {
    std::cout << "Running archive tests... ";

    HRESULT hr;
    sevenzip::Lib l; // not loaded

    // Iarchive: passing nullptr istream should return S_FALSE
    sevenzip::Iarchive iarc(l);
    hr = iarc.open(nullptr, L"file.7z");
    CHECK(hr == S_FALSE, "Iarchive::open with nullptr istream should return S_FALSE");

    // Iarchive: if Open fails on the stream, open should return that HRESULT
    FakeIstream badStream;
    badStream.open_ok = false;
    hr = iarc.open(&badStream, L"file.7z");
    CHECK(hr == S_FALSE, "Iarchive::open should return S_FALSE when stream Open fails");

    // Iarchive: valid stream but lib not initialized -> CreateObjectFunc null -> S_FALSE
    FakeIstream goodStream;
    goodStream.open_ok = true;
    goodStream.seek_ok = true;
    hr = iarc.open(&goodStream, L"file.7z");
    CHECK(hr == S_FALSE, "Iarchive::open should return S_FALSE when library CreateObjectFunc is not available");

    std::cout << "iarchive tests passed." << std::endl;
}
