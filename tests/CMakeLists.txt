set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package (Boost REQUIRED COMPONENTS unit_test_framework)

set(SSU_Sources "range_union_test.cpp")

include_directories(${Boost_INCLUDE_DIR} REQUIRED COMPONENTS unit_test_framework)

add_executable(range_union_test "../range_union.hpp" ${SSU_Sources})
target_link_libraries(range_union_test PRIVATE Boost::unit_test_framework)
add_test(NAME range_union_test COMMAND range_union_test)

target_compile_definitions(range_union_test PRIVATE "-DSOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

if (WIN32)
    target_compile_definitions(range_union_test PRIVATE "-D_WIN32_WINNT=0x0501;WIN32_LEAN_AND_MEAN")
endif()



