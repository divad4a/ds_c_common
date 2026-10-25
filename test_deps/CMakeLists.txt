set(GREATEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/greatest")
if(NOT EXISTS "${GREATEST_DIR}/greatest.h")
    message(FATAL_ERROR "greatest submodule is missing. Run: git submodule update --init --recursive")
endif()

add_library(greatest INTERFACE)
target_include_directories(greatest SYSTEM INTERFACE "${GREATEST_DIR}")
add_library(greatest::greatest ALIAS greatest)