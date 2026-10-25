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
