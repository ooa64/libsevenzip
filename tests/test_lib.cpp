#include <iostream>
#include <cwchar>
#include "sevenzip.h"

static void CHECK(bool cond, const char* msg) {
    if (!cond) {
        std::cerr << "FAIL: " << msg << std::endl;
        throw std::runtime_error(msg);
    }
}

void run_lib_tests() {
    std::cout << "Running lib tests... ";
    sevenzip::Lib l;

    // Loading a non-existing library should fail and provide a message
    bool ok = l.load(L"no_such_library.so");
    CHECK(!ok, "Lib::load should return false for missing library");
    const wchar_t* msg = l.getLoadMessage();
    CHECK(msg && wcslen(msg) > 0, "Lib::getLoadMessage should return a non-empty string when load failed");

    // When library not loaded, these queries should return empty/zero values
    CHECK(l.getVersion() == 0, "Lib::getVersion should be 0 when library not loaded");
    CHECK(l.getNumberOfFormats() == 0, "Lib::getNumberOfFormats should be 0 when library not loaded");
    CHECK(l.getFormatByExtension(L"7z") == -1, "Lib::getFormatByExtension should return -1 when no formats available");

    std::cout << "lib tests passed." << std::endl;
}
