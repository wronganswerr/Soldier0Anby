find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(test_arena test_arena.cpp)
target_link_libraries(test_arena
    PRIVATE
        lsm_engine
        GTest::gtest
        GTest::gtest_main
)

add_executable(test_skiplist test_skiplist.cpp)
target_link_libraries(test_skiplist
    PRIVATE
        lsm_engine
        GTest::gtest
        GTest::gtest_main
)

add_executable(test_memtable test_memtable.cpp)
target_link_libraries(test_memtable
    PRIVATE
        lsm_engine
        GTest::gtest
        GTest::gtest_main
)

add_executable(test_crc32c test_crc32c_test.cpp)
target_link_libraries(test_crc32c
    PRIVATE
        lsm_engine
        GTest::gtest
        GTest::gtest_main
)

gtest_discover_tests(test_arena)
gtest_discover_tests(test_skiplist)
gtest_discover_tests(test_memtable)
gtest_discover_tests(test_crc32c)
