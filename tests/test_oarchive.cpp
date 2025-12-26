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

// Minimal fake Ostream
struct FakeOstream : public sevenzip::Ostream {
    bool open_ok = true;
    bool write_ok = true;
    virtual HRESULT Open(const wchar_t* /*filename*/) override {
        return open_ok ? S_OK : S_FALSE;
    }
    virtual HRESULT Write(const void* /*data*/, UInt32 /*size*/, UInt32& processed) override {
        processed = 0;
        return write_ok ? S_OK : S_FALSE;
    }
    virtual void Close() override {}
};

void run_oarchive_tests() {
    std::cout << "Running archive tests... ";

    HRESULT hr;
    sevenzip::Lib l; // not loaded
    FakeIstream in;
    FakeOstream out;

    // Oarchive: null streams
    sevenzip::Oarchive oarc(l);
    hr = oarc.open(in, out, L"out.7z");
    CHECK(hr == S_FALSE, "Oarchive::open with nullptrs should return S_FALSE");

    // Oarchive: ostream Open failure should propagate
    FakeOstream badO;
    badO.open_ok = false;
    hr = oarc.open(in, badO, L"out.7z");
    CHECK(hr == S_FALSE, "Oarchive::open should return S_FALSE when ostream->Open fails");

    // Oarchive: good streams but library missing -> return S_FALSE (CreateObjectFunc not set)
    FakeOstream goodO;
    goodO.open_ok = true;
    hr = oarc.open(in, goodO, L"out.7z");
    CHECK(hr == S_FALSE, "Oarchive::open should return S_FALSE when library CreateObjectFunc missing");

    std::cout << "oarchive tests passed." << std::endl;
}
