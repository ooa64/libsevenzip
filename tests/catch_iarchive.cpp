#include <catch2/catch.hpp>
#include "catch_tests.h"

using namespace sevenzip;
using namespace sevenzip_test;

TEST_CASE("Iarchive::open handles invalid streams", "[iarchive]") {
    HRESULT hr;
    Lib l; // not loaded

    {
        FakeIstream in;
        Iarchive ar;
        hr = ar.open(l, in, L"file.7z");
        REQUIRE(hr == S_FALSE);
    }

    {
        FakeIstream in;
        Iarchive ar;
        in.open_ok = false;
        hr = ar.open(l, in, L"file.7z");
        REQUIRE(hr == S_FALSE);
    }

    {
        FakeIstream in;
        Iarchive ar;
        in.open_ok = true;
        in.seek_ok = true;
        hr = ar.open(l, in, L"file.7z");
        REQUIRE(hr == S_FALSE);
    }

    {
        FakeIstream in;
        Iarchive ar;
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

    std::string type;

    SECTION("7z format") {type = "7z";}
    SECTION("bz2 format") {type = "bz2";}
    SECTION("gz format") {type = "gz";}
    SECTION("tar format") {type = "tar";}
    SECTION("wim format") {type = "wim";}
    SECTION("xz format") {type = "xz";}
    SECTION("zip format") {type = "zip";}

    std::string base = get_base_dir("iarchive");
    std::string archive = create_7z_archive(exe, base, type);
    if (archive.empty()) {
        WARN("failed to create '" << type << "' test archive using 7z executable");
        return;
    }

    {
        Iarchive ar;
        FileIstream in;

        HRESULT hr = ar.open(lib, in, fromBytes(archive.c_str()));
        REQUIRE(hr == S_OK);

        int count = ar.getNumberOfItems();
        REQUIRE(count >= 1);

        const wchar_t* path0 = ar.getItemPath(0);
        REQUIRE(path0[0] != 0);

        ar.close();
    }
}
