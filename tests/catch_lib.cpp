#include <catch2/catch.hpp>
#include "catch_tests.h"

using namespace sevenzip;
using namespace sevenzip_test;

TEST_CASE("Lib handles missing library", "[lib][load]") {
    Lib l;
    FakeIstream in;
    
    REQUIRE(l.load(L"no_such_library") == false);
    REQUIRE(l.getLoadMessage()[0] != 0);
    REQUIRE(l.isLoaded() == false);
    REQUIRE(l.getVersion() == 0);
    REQUIRE(l.getNumberOfFormats() == 0);
    REQUIRE(l.getFormatByExtension(L"7z") == -1);
    REQUIRE(l.getFormatBySignature(in) == -1);
}

TEST_CASE("Lib loads 7z library", "[lib][load]") {
    Lib lib;
    if (!lib.load(SEVENZIPDLL)) {
        WARN("7z library not available; set LD_LIBRARY_PATH or place 7z.so alongside tests");
        return;
    }

    REQUIRE(lib.isLoaded() != false);
    REQUIRE(lib.getVersion() != 0);
    REQUIRE(lib.getNumberOfFormats() >= 0);
    REQUIRE(lib.getFormatByExtension(L"7z") >= 0);
}

TEST_CASE("Detect format by signature", "[lib][signature]") {
    std::string exe = find_7z_executable();
    if (exe.empty()) {
        WARN("7z executable not available in PATH or SEVENZIP_EXE");
        return;
    }

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

    std::string base = get_base_dir("lib");
    std::string archive = create_7z_archive(exe, base, type);
    if (archive.empty()) {
        WARN("failed to create test '" << type << "' archive using 7z executable");
        return;
    }

    FileIstream stream;
    REQUIRE(stream.openPath(archive));
    int format = lib.getFormatBySignature(stream); 
    REQUIRE(format >= 0);
}
