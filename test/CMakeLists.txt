cmake_minimum_required (VERSION 3.5.1)
project (Range_Union)
enable_testing()

set (targetName "range_union-test")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

link_directories(${Boost_LIBRARY_DIR})
find_package(Boost REQUIRED)

set(SSU_Sources "range_union_test.cpp")

include_directories(${Boost_INCLUDE_DIR} REQUIRED COMPONENTS unit_test_framework)

add_executable(${targetName} "../range_union.hpp" ${SSU_Sources})

add_definitions("-DSOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

if (WIN32)
    add_definitions("-D_WIN32_WINNT=0x0501;WIN32_LEAN_AND_MEAN")
endif()

add_test(NAME tests COMMAND ${targetName})


