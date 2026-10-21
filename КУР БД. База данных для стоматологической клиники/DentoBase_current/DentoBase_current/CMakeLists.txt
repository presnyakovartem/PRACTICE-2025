cmake_minimum_required(VERSION 3.14)

project(DentoBase VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist/components)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist/components)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)

find_package(PkgConfig REQUIRED)

pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(PQ REQUIRED libpq)
pkg_check_modules(HIREDIS REQUIRED hiredis)

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${PQXX_INCLUDE_DIRS}
    ${PQ_INCLUDE_DIRS}
    ${HIREDIS_INCLUDE_DIRS}
)

link_directories(
    ${PQXX_LIBRARY_DIRS}
    ${PQ_LIBRARY_DIRS}
    ${HIREDIS_LIBRARY_DIRS}
)

add_library(dentobase_config STATIC
    src/config/Config.cpp
)

target_include_directories(dentobase_config
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

add_library(dentobase_logging STATIC
    src/logging/Logger.cpp
)

target_include_directories(dentobase_logging
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(dentobase_logging
    PUBLIC
        spdlog::spdlog
        fmt::fmt
)

add_library(dentobase_cache STATIC
    src/cache/RedisCache.cpp
)

target_include_directories(dentobase_cache
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${HIREDIS_INCLUDE_DIRS}
)

target_link_libraries(dentobase_cache
    PUBLIC
        dentobase_logging
        ${HIREDIS_LIBRARIES}
)

add_library(dentobase_data_access STATIC
    src/sql_builders/postgres/PostgresPatientSqlBuilder.cpp
    src/sql_builders/postgres/PostgresDoctorSqlBuilder.cpp
    src/sql_builders/postgres/PostgresMedicalRecordSqlBuilder.cpp
    src/sql_builders/postgres/PostgresImageSqlBuilder.cpp
    src/sql_builders/postgres/PostgresDiagnosisSqlBuilder.cpp

    src/repositories/postgres/PostgresPatientRepository.cpp
    src/repositories/postgres/PostgresDoctorRepository.cpp
    src/repositories/postgres/PostgresMedicalRecordRepository.cpp
    src/repositories/postgres/PostgresImageRepository.cpp
    src/repositories/postgres/PostgresDiagnosisRepository.cpp
)

target_include_directories(dentobase_data_access
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${PQXX_INCLUDE_DIRS}
        ${PQ_INCLUDE_DIRS}
)

target_link_libraries(dentobase_data_access
    PUBLIC
        ${PQXX_LIBRARIES}
        ${PQ_LIBRARIES}
)

add_library(dentobase_business STATIC
    src/services/ImageService.cpp
    src/services/DiagnosisService.cpp
    src/app/ApplicationService.cpp
)

target_include_directories(dentobase_business
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${PQXX_INCLUDE_DIRS}
        ${PQ_INCLUDE_DIRS}
)

target_link_libraries(dentobase_business
    PUBLIC
        dentobase_data_access
        dentobase_logging
        dentobase_cache
        ${PQXX_LIBRARIES}
        ${PQ_LIBRARIES}
)

add_library(dentobase_gui STATIC
    src/gui/MainWindow.cpp
)

target_include_directories(dentobase_gui
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${PQXX_INCLUDE_DIRS}
        ${PQ_INCLUDE_DIRS}
        ${HIREDIS_INCLUDE_DIRS}
)

target_link_libraries(dentobase_gui
    PUBLIC
        Qt6::Widgets
        dentobase_business
        dentobase_cache
        dentobase_logging
)

add_executable(dentobase_gui_app
    src/gui/GuiMain.cpp
)

target_include_directories(dentobase_gui_app
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${PQXX_INCLUDE_DIRS}
        ${PQ_INCLUDE_DIRS}
        ${HIREDIS_INCLUDE_DIRS}
)

target_link_libraries(dentobase_gui_app
    PRIVATE
        dentobase_config
        dentobase_logging
        dentobase_cache
        dentobase_data_access
        dentobase_business
        dentobase_gui

        Qt6::Widgets
        spdlog::spdlog
        fmt::fmt

        ${PQXX_LIBRARIES}
        ${PQ_LIBRARIES}
        ${HIREDIS_LIBRARIES}
)
