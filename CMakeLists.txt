cmake_minimum_required(VERSION 3.28)

project(ds_c_common LANGUAGES C VERSION 0.0.1 DESCRIPTION "Personal common C utilities")

include(CTest)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(ds_c_common)

target_sources(ds_c_common PRIVATE
    src/vector.c
)
target_include_directories(ds_c_common PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(ds_c_common::ds_c_common ALIAS ds_c_common)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()


