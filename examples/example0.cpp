#include <fstream>
#include <iostream>
#include "sevenzip.h"

using namespace std;
using namespace sevenzip;

int main() {
    setlocale(LC_ALL, "");
    wcout << "Library formats\n\n";

    Lib l;
    if (!l.load(SEVENZIPDLL)) {
        wcout << l.getLoadMessage() << "\n";
        return 1;
    }

    wcout << "client version : " << (getVersion() >> 16) << "." << (getVersion() & 0xffff) << "\n";
    wcout << "library version : " << (l.getVersion() >> 16) << "." << (l.getVersion() & 0xffff) << "\n";
    wcout << "formats :\n";
    int n = l.getNumberOfFormats();
    for (int i = 0; i < n; i++) {
        wcout << i+1 << " : "
            << l.getFormatName(i) << (l.getFormatUpdatable(i) ? "(updatable)" : "") << ": "
            << l.getFormatExtensions(i) << "\n";
    }

    if (n > 0) {
        wcout << "TEST PASSED\n";
    }
    return 0;
}
