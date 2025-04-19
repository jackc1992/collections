CMAKE_MINIMUM_REQUIRED(VERSION 3.30)

find_package(Boost 1.88 REQUIRED lockfree)
find_package(folly CONFIG REQUIRED)
find_package(gflags CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)


set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable Google Benchmark's own tests" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Disable installing Google Benchmark" FORCE)
set(BENCHMARK_ENABLE_EXCEPTIONS OFF CACHE BOOL "" FORCE)

SET(BENCHMARK_SOURCES src/spsc_bm.cpp src/simple_bm.cpp)

ADD_EXECUTABLE(my_benchmarks ${BENCHMARK_SOURCES})

SET_TARGET_PROPERTIES(my_benchmarks PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD})
SET_TARGET_PROPERTIES(my_benchmarks PROPERTIES CXX_STANDARD_REQUIRED ${CMAKE_CXX_STANDARD_REQUIRED})
SET_TARGET_PROPERTIES(my_benchmarks PROPERTIES CXX_EXTENSIONS ${CMAKE_CXX_EXTENSIONS})

TARGET_LINK_LIBRARIES(my_benchmarks PRIVATE
    jc_collections
    benchmark::benchmark
)

IF(TARGET benchmark::benchmark)
    add_test(NAME my_benchmarks_run COMMAND my_benchmarks)
ENDIF()
