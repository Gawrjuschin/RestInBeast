cmake_minimum_required(VERSION 3.23)

project(
    rest_in_beast
    VERSION 0.1.0
    DESCRIPTION "Boost Beast examples inspired multi thread async http server library"
    LANGUAGES CXX
)

option(REST_IN_BEAST_BUILD_TESTS "" ON)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Dependencies
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_package(
    OpenSSL 1.0.2 # beast requirements
    REQUIRED
)

find_package(
    Boost 1.70 # boost::beast
    CONFIG  # boost config support for new CMake
    COMPONENTS system regex asio beast unit_test_framework
    REQUIRED
)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Library
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
add_library(rest_in_beast_server INTERFACE)
add_library(rest_in_beast::server ALIAS rest_in_beast_server)

set_target_properties(
    rest_in_beast_server PROPERTIES
    # OUTPUT_NAME не имеет смысла менять, так как нет бинаря\
    # EXPORT_NAME указывается для экспорта с NAMESPACE
        EXPORT_NAME server
)

target_sources(
    rest_in_beast_server
    PUBLIC
    # Новая фича, которая позволяет отслеживать заголовочные файлы в IDE и не заморачиваться с include_directories и install
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include
    FILES
    ${CMAKE_CURRENT_LIST_DIR}/include/rest_in_beast/server.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/rest_in_beast/template.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/rest_in_beast/detail/logger.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/rest_in_beast/detail/respondent.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/rest_in_beast/detail/session.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/rest_in_beast/detail/template_iterator.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/rest_in_beast/util/hasher.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/rest_in_beast/util/shared_proxy.hpp
)

target_link_libraries(
    rest_in_beast_server
    INTERFACE
    Boost::asio
    Boost::beast
    OpenSSL::SSL
    OpenSSL::Crypto
)

# TODO: find sockets
if(WIN32)
    target_link_libraries(
        rest_in_beast_server
        INTERFACE
        wsock32 ws2_32
    )
endif()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Testing
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if(PROJECT_IS_TOP_LEVEL AND REST_IN_BEAST_BUILD_TESTS)
    include(CTest)
    enable_testing()


    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # sercer test
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    add_executable(rest_in_beast_server_tests)
    add_test(NAME serverTests COMMAND $<TARGET_FILE:rest_in_beast_server_tests>)
    target_sources(
        rest_in_beast_server_tests
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/test/server.cpp
        ${CMAKE_CURRENT_LIST_DIR}/test/support/test_requests.hpp
        ${CMAKE_CURRENT_LIST_DIR}/test/support/test_logger.hpp
        ${CMAKE_CURRENT_LIST_DIR}/test/support/test_respondent.hpp
        ${CMAKE_CURRENT_LIST_DIR}/test/support/test_signal_handler.hpp
        ${CMAKE_CURRENT_LIST_DIR}/test/support/test_asio_thread.hpp
        ${CMAKE_CURRENT_LIST_DIR}/test/support/test_clients.hpp
    )

target_link_libraries(
    rest_in_beast_server_tests
    PRIVATE
    Boost::unit_test_framework
    rest_in_beast::server
)

target_compile_features(
    rest_in_beast_server_tests
    PRIVATE
    cxx_std_20
)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# template test
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
add_executable(rest_in_beast_template_test)
add_test(NAME templateTest COMMAND $<TARGET_FILE:rest_in_beast_template_test>)

target_sources(
    rest_in_beast_template_test
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/test/template.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/rest_in_beast/template.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/rest_in_beast/detail/template_iterator.hpp
)

target_link_libraries(
    rest_in_beast_template_test
    PRIVATE
    Boost::unit_test_framework
    rest_in_beast::server
)

target_compile_features(
    rest_in_beast_template_test
    PRIVATE
    cxx_std_20
)

endif()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Packaging
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
include(GNUInstallDirs)

install(
    TARGETS rest_in_beast_server
    EXPORT ${PROJECT_NAME}-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT ${PROJECT_NAME}-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    NAMESPACE ${PROJECT_NAME}::
    FILE ${PROJECT_NAME}-config.cmake
)
