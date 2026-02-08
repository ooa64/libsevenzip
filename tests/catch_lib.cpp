#include <catch2/catch.hpp>

#include "catch_tests.h"

using namespace sevenzip;
using namespace sevenzip_test;

TEST_CASE("Lib handles missing library", "[lib]") {
    Lib l;
    FakeIstream in;
    
    REQUIRE(l.load(L"no_such_library") == false);
    REQUIRE(l.getLoadMessage() != nullptr);
    REQUIRE(l.getVersion() == 0);
    REQUIRE(l.getNumberOfFormats() == 0);
    REQUIRE(l.getFormatByExtension(L"7z") == -1);
    REQUIRE(l.getFormatBySignature(in) == -1);
}

TEST_CASE("Lib loads 7z library", "[lib]") {
    Lib lib;
    if (!lib.load(SEVENZIPDLL)) {
        WARN("7z library not available; set LD_LIBRARY_PATH or place 7z.so alongside tests");
        return;
    }

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

    std::string base = get_base_dir("lib");
    std::string archive = create_7z_archive(exe, base);
    if (archive.empty()) {
        WARN("failed to create test archive using 7z executable");
        return;
    }

    FileIstream stream;
    REQUIRE(stream.openPath(archive));
    int format = lib.getFormatBySignature(stream);
    REQUIRE(format >= 0);
}
