cmake_minimum_required(VERSION 3.21)
project(sevenzip LANGUAGES C CXX)

if(NOT SEVENZIPSRC)
    set(SEVENZIPSRC $ENV{SEVENZIPSRC})
endif()

if(NOT SEVENZIPSRC)
    set(SEVENZIPSRC "../7zip")
endif()

# add_compile_definitions(UNICODE _UNICODE)
# add_compile_definitions(DEBUG_IMPL)
# add_compile_options(-fsanitize=address -Zi)

add_library(sevenzip STATIC "sevenzip.cpp" "sevenzip_impl.cpp"
    "${SEVENZIPSRC}/CPP/Common/MyWindows.cpp"
    "${SEVENZIPSRC}/CPP/Common/MyString.cpp"
    "${SEVENZIPSRC}/CPP/Common/IntToString.cpp"
    "${SEVENZIPSRC}/CPP/Common/StringConvert.cpp"
    "${SEVENZIPSRC}/CPP/Common/UTFConvert.cpp"
    "${SEVENZIPSRC}/CPP/Windows/PropVariant.cpp"
    "${SEVENZIPSRC}/CPP/Windows/TimeUtils.cpp"
    "${SEVENZIPSRC}/CPP/Windows/ErrorMsg.cpp"
    "${SEVENZIPSRC}/CPP/Windows/DLL.cpp"
)
target_include_directories(sevenzip PUBLIC ${SEVENZIPSRC})

add_executable (example "examples/example.cpp" "sevenzip.h")
target_include_directories(example PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(example sevenzip)

foreach(i RANGE 1 9)
    set(target "example${i}")
    add_executable(${target} "examples/${target}.cpp")
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(${target} PRIVATE sevenzip)
endforeach()

add_executable(tests
    tests/tests.cpp
    tests/test_lib.cpp
    tests/test_iarchive.cpp
    tests/test_oarchive.cpp
)
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(tests PRIVATE sevenzip)
add_test(NAME tests COMMAND tests)

enable_testing()
