#include <catch2/catch.hpp>
#include "catch_tests.h"

using namespace sevenzip;
using namespace sevenzip_test;

TEST_CASE ("Oarchive::open handles invalid streams", "[oarchive]") {
    HRESULT hr;
    Lib l;

    {
        Oarchive ar;
        FakeIstream in;
        FakeOstream out;
        hr = ar.open(l, in, out, L"out.7z");
        REQUIRE(hr == S_FALSE);
    }

    {
        Oarchive ar;
        FakeIstream in;
        FakeOstream out;
        out.open_ok = false;
        hr = ar.open(l, in, out, L"out.7z");
        REQUIRE(hr == S_FALSE);
    }

    {
        Oarchive ar;
        FakeIstream in;
        FakeOstream out;
        out.open_ok = true;
        hr = ar.open(l, in, out, L"out.7z");
        REQUIRE(hr == S_FALSE);
    }
}

TEST_CASE("Create archive with single file and reopen", "[oarchive]") {
    Lib lib;
    if (!lib.load(SEVENZIPDLL)) {
        WARN("7z library not available; set LD_LIBRARY_PATH or place 7z.so alongside tests");
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

    INFO("Testing format: " << type);

    std::string base = get_base_dir("oarchive");
    std::string archive = "out." + type;

    if (!write_text_file(base + "/in.txt", "libsevenzip out content")) {
        FAIL("failed to create input file for oarchive");
    }

    if (path_exists(base + "/" + archive)) {
        remove_file(base + "/" + archive);
    }

    HRESULT hr;
    wchar_t wArchive[1024];
    fromBytes(wArchive, 1024, archive.c_str());

    {
        Oarchive ar;
        FileIstream in(base);
        FileOstream out(base);

        hr = ar.open(lib, in, out, wArchive);
        REQUIRE(hr == S_OK);

        ar.addItem(fromBytes("in.txt"));
        hr = ar.update();
        REQUIRE(hr == S_OK);

        ar.close();
    }

    {
        Iarchive ar;
        FileIstream in(base);
        hr = ar.open(lib, in, wArchive);
        REQUIRE(hr == S_OK);

        int count = ar.getNumberOfItems();
        REQUIRE(count >= 1);

        ar.close();
    }
}

TEST_CASE("Create archive with multiple files and reopen", "[oarchive]") {
    Lib lib;
    if (!lib.load(SEVENZIPDLL)) {
        WARN("7z library not available; set LD_LIBRARY_PATH or place 7z.so alongside tests");
        return;
    }

    std::string type;

    SECTION("7z format") {type = "7z";}
    SECTION("tar format") {type = "tar";}
    SECTION("wim format") {type = "wim";}
    SECTION("zip format") {type = "zip";}

    INFO("Testing format: " << type);

    std::string base = get_base_dir("oarchive");
    std::string archive = "outs." + type;

    for (int i = 0; i < 10; i++) {
        std::string input = base + "/in" + std::to_string(i) + ".txt";
        if (!write_text_file(input, "libsevenzip out content " + std::to_string(i))) {
            FAIL("failed to create input file for oarchive");
        }
    }

    if (path_exists(base + "/" + archive)) {
        remove_file(base + "/" + archive);
    }

    HRESULT hr;
    wchar_t wArchive[1024];
    fromBytes(wArchive, 1024, archive.c_str());

    {
        Oarchive ar;
        FileIstream in(base);
        FileOstream out(base);

        hr = ar.open(lib, in, out, wArchive);
        REQUIRE(hr == S_OK);

        for (int i = 0; i < 10; i++) {
            std::string input = "in" + std::to_string(i) + ".txt";
            ar.addItem(fromBytes(input.c_str()));
        }
        hr = ar.update();
        REQUIRE(hr == S_OK);    

        ar.close();
    }

    {
        Iarchive ar;
        FileIstream in(base);
        hr = ar.open(lib, in, wArchive);
        REQUIRE(hr == S_OK);

        int count = ar.getNumberOfItems();
        REQUIRE(count >= 10);

        ar.close();
    }
}

TEST_CASE("Create archive with mt option files and reopen", "[oarchive]") {
    Lib lib;
    if (!lib.load(SEVENZIPDLL)) {
        WARN("7z library not available; set LD_LIBRARY_PATH or place 7z.so alongside tests");
        return;
    }

    std::string type;

    SECTION("7z format") {type = "7z";}
    SECTION("tar format") {type = "tar";}
    SECTION("wim format") {type = "wim";}
    SECTION("zip format") {type = "zip";}

    INFO("Testing format: " << type);

    std::string base = get_base_dir("oarchive");
    std::string archive = "outs." + type;

    for (int i = 0; i < 10; i++) {
        std::string input = base + "/in" + std::to_string(i) + ".txt";
        if (!write_text_file(input, "libsevenzip out content " + std::to_string(i))) {
            FAIL("failed to create input file for oarchive");
        }
    }

    if (path_exists(archive)) {
        remove_file(archive);
    }

    HRESULT hr;
    wchar_t wArchive[1024];
    fromBytes(wArchive, 1024, archive.c_str());

    {
        Oarchive ar;
        FileIstream in(base);
        FileOstream out(base);

        hr = ar.open(lib, in, out, wArchive);
        REQUIRE(hr == S_OK);

        for (int i = 0; i < 10; i++) {
            std::string input = "in" + std::to_string(i) + ".txt";
            ar.addItem(fromBytes(input.c_str()));
        }
        ar.addIntOption(L"mt", 64); 
        hr = ar.update();
        REQUIRE(hr == S_OK);

        ar.close();
    }

    {
        Iarchive ar;
        FileIstream in(base);
        FileOstream out(base);
        hr = ar.open(lib, in, wArchive);
        REQUIRE(hr == S_OK);

        int count = ar.getNumberOfItems();
        REQUIRE(count >= 10);

        ar.addIntOption(L"mt", 64);
        hr = ar.extract(out);
        REQUIRE(hr == S_OK);

        ar.close();
    }
}
