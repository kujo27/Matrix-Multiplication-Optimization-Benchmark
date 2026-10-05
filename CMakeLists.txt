cmake_minimum_required(VERSION 4.4.2)
project(matrix_demo CXX)

# Set C++23 Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(matrix_demo  src/main.cpp)

target_compile_options(matrix_demo PRIVATE
    -fdiagnostics-color=always
    -ggdb
    -pedantic-errors
    -Wall
    -Weffc++
    -Wextra
    -Wconversion
    -Wsign-conversion
    -Werror
)

include(FetchContent)
set(BENCHMARK_ENABLE_TESTING OFF)

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.4
)

FetchContent_MakeAvailable(googlebenchmark)

add_executable(matrix_benchmark src/benchmarks.cpp)


target_link_libraries(matrix_benchmark PRIVATE benchmark::benchmark_main)


find_package(OpenMP REQUIRED)

target_link_libraries(matrix_demo PRIVATE OpenMP::OpenMP_CXX)
target_include_directories(matrix_demo PRIVATE include)

target_link_libraries(matrix_benchmark PRIVATE OpenMP::OpenMP_CXX)
target_include_directories(matrix_benchmark PRIVATE include)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(nlohmann_json)

add_executable(matrix_analysis src/analyze_results.cpp)

target_link_libraries( matrix_analysis PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(matrix_analysis PRIVATE include)
