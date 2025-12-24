#include <iostream>

void run_lib_tests();
void run_iarchive_tests();
void run_oarchive_tests();

int main() {
    int failures = 0;
    try {
        run_lib_tests();
    } catch (const std::exception& e) {
        std::cerr << "Exception in lib tests: " << e.what() << std::endl;
        ++failures;
    }
    try {
        run_iarchive_tests();
    } catch (const std::exception& e) {
        std::cerr << "Exception in iarchive tests: " << e.what() << std::endl;
        ++failures;
    }
    try {
        run_oarchive_tests();
    } catch (const std::exception& e) {
        std::cerr << "Exception in oarchive tests: " << e.what() << std::endl;
        ++failures;
    }

    if (failures) {
        std::cerr << failures << " test group(s) failed." << std::endl;
        return 1;
    }

    std::cout << "All tests passed." << std::endl;
    return 0;
}
