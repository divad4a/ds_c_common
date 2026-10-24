# greatest is a single-header test framework, vendored as a git submodule.
set(GREATEST_DIR "${PROJECT_SOURCE_DIR}/test_deps/greatest")
if(NOT EXISTS "${GREATEST_DIR}/greatest.h")
    message(FATAL_ERROR "greatest not found in ${GREATEST_DIR}. Run: git submodule update --init --recursive")
endif()

add_library(greatest INTERFACE)
target_include_directories(greatest SYSTEM INTERFACE "${GREATEST_DIR}")
add_library(greatest::greatest ALIAS greatest)

add_executable(ds_c_common_test_vector
    test_vector.c
)

target_link_libraries(ds_c_common_test_vector
    PRIVATE
        ds_c_common::ds_c_common
        greatest::greatest
)

# Register each greatest suite as its own CTest test (selected via greatest's "-s <suite>" flag).
set(VECTOR_TEST_SUITES
    vector_lifecycle
    vector_capacity
    vector_modifiers
    vector_access
    vector_element_types
)
foreach(suite IN LISTS VECTOR_TEST_SUITES)
    add_test(NAME ds_c_common.vector.${suite} COMMAND ds_c_common_test_vector -v -e -s ${suite})
    set_tests_properties(ds_c_common.vector.${suite} PROPERTIES TIMEOUT 10)
endforeach()

# Isolate regressions that may hang or abort in the current implementation.
set(VECTOR_REGRESSION_TESTS
    push_past_reserved_capacity_one
    push_after_shrinking_to_one
    shrink_allocated_empty_vector
)
foreach(test IN LISTS VECTOR_REGRESSION_TESTS)
    add_test(NAME ds_c_common.vector.${test} COMMAND ds_c_common_test_vector -v -e -t ${test})
    set_tests_properties(ds_c_common.vector.${test} PROPERTIES TIMEOUT 5)
endforeach()

# Let overflowing allocation requests return NULL under ASan, as they do without it.
set_tests_properties(ds_c_common.vector.vector_capacity PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=allocator_may_return_null=1"
)
