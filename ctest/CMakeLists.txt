find_package(GTest REQUIRED)
include(GoogleTest)

# 先让最小可用的单测跑起来：只编译 arena 相关代码，避免被 kvengine 其它未完成模块阻塞。
add_library(test_only STATIC
    ${PROJECT_SOURCE_DIR}/src/arena.cpp
    ${PROJECT_SOURCE_DIR}/src/skiplist.hpp
    ${PROJECT_SOURCE_DIR}/src/memtable.cpp

    # 下面这些是 memtable 单测所需的最小实现集合
    ${PROJECT_SOURCE_DIR}/src/comparator.cpp
    ${PROJECT_SOURCE_DIR}/src/internal_key_comparator.cpp
    ${PROJECT_SOURCE_DIR}/src/look_up_key.cpp
    ${PROJECT_SOURCE_DIR}/src/internal_key.cpp
    ${PROJECT_SOURCE_DIR}/src/db_format.cpp
    ${PROJECT_SOURCE_DIR}/src/status.cpp
    ${PROJECT_SOURCE_DIR}/src/coding.cpp
    ${PROJECT_SOURCE_DIR}/src/iterator.cpp

    # crc32c 单测所需
    ${PROJECT_SOURCE_DIR}/src/crc32c.cpp
)

target_compile_features(test_only PUBLIC cxx_std_20)
target_include_directories(test_only PUBLIC ${PROJECT_SOURCE_DIR}/src)


# ------------------------------------------------------------
# 工程化注册测试：新增测试时“显式列出目标”（便于 code review/可控），
# 同时通过一个函数去掉重复样板代码。
# 约定：源文件名与 target 同名，例如：add_unit_test(test_arena) 对应 test_arena.cpp
# ------------------------------------------------------------
function(add_unit_test name)
    add_executable("${name}" "${name}.cpp")
    target_link_libraries("${name}"
        PRIVATE
            test_only
            GTest::gtest
            GTest::gtest_main
    )
    add_test(NAME "${name}" COMMAND "${name}")
endfunction()

add_unit_test(test_arena)
add_unit_test(test_skiplist)
add_unit_test(test_memtable)
add_unit_test(test_crc32c_test)

# 未来如果想恢复/加入 crc32c 测试：
# add_unit_test(test_crc32c_test)
