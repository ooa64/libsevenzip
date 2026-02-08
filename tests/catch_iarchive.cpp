#include <catch2/catch.hpp>

#include <string>

#include "catch_tests.h"

using namespace sevenzip;
using namespace sevenzip_test;

TEST_CASE("Iarchive::open handles invalid streams", "[iarchive]") {
    HRESULT hr;
    sevenzip::Lib l; // not loaded

    {
        FakeIstream in;
        sevenzip::Iarchive ar;
        hr = ar.open(l, in, L"file.7z");
        REQUIRE(hr == S_FALSE);
    }

    {
        FakeIstream in;
        sevenzip::Iarchive ar;
        in.open_ok = false;
        hr = ar.open(l, in, L"file.7z");
        REQUIRE(hr == S_FALSE);
    }

    {
        FakeIstream in;
        sevenzip::Iarchive ar;
        in.open_ok = true;
        in.seek_ok = true;
        hr = ar.open(l, in, L"file.7z");
        REQUIRE(hr == S_FALSE);
    }

    {
        FakeIstream in;
        sevenzip::Iarchive ar;
        in.open_ok = false;
        hr = ar.open(l, in, L"file.7z");
        REQUIRE(hr == S_FALSE);
    }
}

TEST_CASE("Open archive created by 7z", "[iarchive]") {
    std::string exe = find_7z_executable();
    if (exe.empty()) {
        WARN("7z executable not available in PATH or SEVENZIPBIN");
        return;
    }

    Lib lib;
    if (!lib.load(SEVENZIPDLL)) {
        WARN("7z library not available; set (DY)LD_LIBRARY_PATH");
        return;
    }

    std::string base = get_base_dir("iarchive");
    std::string archive = create_7z_archive(exe, base);
    if (archive.empty()) {
        WARN("failed to create test archive using 7z executable");
        return;
    }

    Iarchive ar;
    FileIstream stream;

    wchar_t wArchive[1024];
    fromBytes(wArchive, 1024, archive.c_str());

    HRESULT hr = ar.open(lib, stream, wArchive);
    REQUIRE(hr == S_OK);

    int count = ar.getNumberOfItems();
    REQUIRE(count > 0);

    const wchar_t* path0 = ar.getItemPath(0);
    REQUIRE(path0 != nullptr);

    ar.close();
}
