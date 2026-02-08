cmake_minimum_required(VERSION 3.20)
project(sevenzip LANGUAGES C CXX)

include(FetchContent)

if(NOT SEVENZIPSRC)
    set(SEVENZIPSRC $ENV{SEVENZIPSRC})
endif()

if(NOT SEVENZIPSRC)
    set(SEVENZIPSRC "../7zip")
endif()

add_compile_definitions(PUBLIC PROJECT_VER_MAJOR=1)
add_compile_definitions(PUBLIC PROJECT_VER_MINOR=1)
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

foreach(i RANGE 0 9)
    set(target "example${i}")
    add_executable(${target} "examples/${target}.cpp")
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(${target} PRIVATE sevenzip)
endforeach()

FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v2.13.10)
FetchContent_MakeAvailable(Catch2)

add_executable(catch_tests
    tests/catch_tests.cpp
    tests/catch_lib.cpp
    tests/catch_iarchive.cpp
    tests/catch_oarchive.cpp
)
target_include_directories(catch_tests PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(catch_tests PRIVATE sevenzip Catch2::Catch2)
target_compile_features(catch_tests PRIVATE cxx_std_11)

add_test(NAME catch_tests COMMAND catch_tests)

enable_testing()
