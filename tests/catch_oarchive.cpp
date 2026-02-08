#include <catch2/catch.hpp>

#include <string>

#include "catch_tests.h"

using namespace sevenzip;
using namespace sevenzip_test;

TEST_CASE ("Oarchive::open handles invalid streams", "[oarchive]") {
    HRESULT hr;
    sevenzip::Lib l;

    {
        FakeIstream in;
        FakeOstream out;
        sevenzip::Oarchive oarc;
        hr = oarc.open(l, in, out, L"out.7z");
        REQUIRE(hr == S_FALSE);
    }

    {
        FakeIstream in;
        FakeOstream out;
        sevenzip::Oarchive oarc;
        out.open_ok = false;
        hr = oarc.open(l, in, out, L"out.7z");
        REQUIRE(hr == S_FALSE);
    }

    {
        FakeIstream in;
        FakeOstream out;
        sevenzip::Oarchive oarc;
        out.open_ok = true;
        hr = oarc.open(l, in, out, L"out.7z");
        REQUIRE(hr == S_FALSE);
    }
}

TEST_CASE("Create archive with Oarchive and reopen", "[oarchive]") {
    Lib lib;
    if (!lib.load(SEVENZIPDLL)) {
        WARN("7z library not available; set LD_LIBRARY_PATH or place 7z.so alongside tests");
        return;
    }

    std::string base = get_base_dir("oarchive");
    std::string input = base + "/input.txt";
    std::string archive = base + "/out.7z";

    if (!write_text_file(input, "oarchive content")) {
        FAIL("failed to create input file for oarchive");
    }

    if (path_exists(archive)) {
        remove_file(archive);
    }

    FileIstream istream;
    FileOstream ostream;

    wchar_t wInput[1024];
    wchar_t wArchive[1024];
    fromBytes(wInput, 1024, input.c_str());
    fromBytes(wArchive, 1024, archive.c_str());

    Oarchive oarc;
    HRESULT hr = oarc.open(lib, istream, ostream, wArchive);
    REQUIRE(hr == S_OK);

    oarc.addItem(wInput);
    hr = oarc.update();
    REQUIRE(hr == S_OK);
    oarc.close();

    Iarchive iarc;
    FileIstream in;
    hr = iarc.open(lib, in, wArchive);
    REQUIRE(hr == S_OK);

    int count = iarc.getNumberOfItems();
    REQUIRE(count > 0);

    const wchar_t* path0 = iarc.getItemPath(0);
    REQUIRE(path0 != nullptr);

    iarc.close();
}
