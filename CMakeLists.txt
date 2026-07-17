cmake_minimum_required(VERSION 3.14)
project(PropMixrTests)

# Automatically fetch GoogleTest from GitHub
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Include your main project target/headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(run_tests 
    tests/test_parser.cpp 
    tests/test_pantry.cpp 
    tests/test_engine.cpp
)

target_link_libraries(run_tests gtest_main ryml pugixml)
include(GoogleTest)
gtest_discover_tests(run_tests)