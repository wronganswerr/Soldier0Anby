cmake_minimum_required(VERSION 3.20)

project(Soldier0Anby
    VERSION 0.1.0
    DESCRIPTION "A key-value storage engine"
    LANGUAGES CXX
)

# 让下游或打包方决定默认构建 STATIC 还是 SHARED
option(BUILD_SHARED_LIBS "Build shared libraries by default" ON)

# 是否构建独立进程模式
option(BUILD_SERVER "Build kvengine standalone server" ON)

# 是否构建测试
option(BUILD_TESTS "Build tests" ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT BUILD_TESTS)
    # -------------------------
    # 核心库
    # TODO: 源文件列表可能需要更改
    # -------------------------
    add_library(kvengine
		src/arena.cpp
		src/cache.cpp
		src/coding.cpp
		src/comparator.cpp
		src/crc32c.cpp
		src/db_format.cpp
		src/env.cpp
		src/env_posix.cpp
		src/filename.cpp
		src/filter_block.cpp
		src/filter_policy.cpp
		src/hash.cpp
		src/interator.cpp
		src/internal_filter_policy.cpp
		src/internal_key.cpp
		src/internal_key_comparator.cpp
		src/logging.cpp
		src/look_up_key.cpp
		src/memtable.cpp
		# src/my_logger.cpp
		src/options.cpp
		src/status.cpp
		src/table.cpp
		src/table_builder.cpp
		src/table_cache.cpp
		# src/thread_pool.cpp
		# src/tool.cpp
		src/version_edit.cpp
		src/version_set.cpp
		src/write_batch.cpp
		src/zero_db.cpp
	)

    add_library(Soldier0Anby::kvengine ALIAS kvengine)

    target_compile_features(kvengine PUBLIC cxx_std_20)

    target_include_directories(kvengine
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    # -------------------------
    # 独立进程 target
    # -------------------------
    if(BUILD_SERVER)
        add_executable(kvengine_server
            src/server_main.cpp
        )

        target_link_libraries(kvengine_server
            PRIVATE
                kvengine
        )
    endif()

    # -------------------------
    # 安装头文件
    # -------------------------
    install(
        DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # -------------------------
    # 安装 target，并把它们关联到导出集
    # -------------------------
    install(
        TARGETS kvengine
        EXPORT Soldier0AnbyTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    if(BUILD_SERVER)
        install(
            TARGETS kvengine_server
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()

    # 安装导出文件（给 find_package 用）
    install(
        EXPORT Soldier0AnbyTargets
        FILE Soldier0AnbyTargets.cmake
        NAMESPACE Soldier0Anby::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Soldier0Anby
    )
    # -------------------------
    # 生成并安装 Config / ConfigVersion
    # 把你的库包装成一个“能被别的 CMake 项目用 find_package(Soldier0Anby CONFIG REQUIRED) 找到并正确链接”的标准包。 -- GPT
    # -------------------------
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Soldier0AnbyConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/Soldier0AnbyConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Soldier0Anby
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/Soldier0AnbyConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/Soldier0AnbyConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/Soldier0AnbyConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Soldier0Anby
    )

    # -------------------------
    # 可选：导出 build tree，方便不安装直接给其他 CMake 工程引用
    # -------------------------
    export(
        EXPORT Soldier0AnbyTargets
        FILE ${CMAKE_CURRENT_BINARY_DIR}/Soldier0AnbyTargets.cmake
        NAMESPACE Soldier0Anby::
    )
endif()

# -------------------------
# 测试
# -------------------------
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(ctest)
endif()

