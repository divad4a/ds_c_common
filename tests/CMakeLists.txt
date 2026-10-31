add_executable(ds_c_common_test_dsc_vector
    test_dsc_vector.c
)

target_link_libraries(ds_c_common_test_dsc_vector
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
    add_test(NAME ds_c_common.vector.${suite} COMMAND ds_c_common_test_dsc_vector -v -e -s ${suite})
    set_tests_properties(ds_c_common.vector.${suite} PROPERTIES TIMEOUT 10)
endforeach()

# Let overflowing allocation requests return NULL under ASan, as they do without it.
set_tests_properties(ds_c_common.vector.vector_capacity PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=allocator_may_return_null=1"
)

# Build the shared helpers with test allocators for deterministic failures.
add_library(ds_c_common_test_typed_vector_helpers OBJECT
    ${PROJECT_SOURCE_DIR}/src/dsc_typed_vector.c
)
target_include_directories(ds_c_common_test_typed_vector_helpers PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_compile_definitions(ds_c_common_test_typed_vector_helpers PRIVATE
    dsc_typed_vector_private_scaled_capacity=typed_vector_test_private_scaled_capacity
    dsc_typed_vector_private_reallocate=typed_vector_test_private_reallocate
    realloc=typed_vector_test_realloc
    aligned_alloc=typed_vector_test_aligned_alloc
)

add_executable(ds_c_common_test_dsc_typed_vector
    test_dsc_typed_vector.c
    typed_vector_fixtures.c
)

target_link_libraries(ds_c_common_test_dsc_typed_vector
    PRIVATE
        ds_c_common::ds_c_common
        ds_c_common_test_typed_vector_helpers
        greatest::greatest
)

set(TYPED_VECTOR_TEST_SUITES
    typed_vector_lifecycle
    typed_vector_capacity
    typed_vector_modifiers
    typed_vector_access
    typed_vector_element_types
    typed_vector_alignment
    typed_vector_arithmetic
)
foreach(suite IN LISTS TYPED_VECTOR_TEST_SUITES)
    add_test(NAME ds_c_common.typed_vector.${suite} COMMAND ds_c_common_test_dsc_typed_vector -v -e -s ${suite})
    set_tests_properties(ds_c_common.typed_vector.${suite} PROPERTIES TIMEOUT 10)
endforeach()
