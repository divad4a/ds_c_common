#include "typed_vector_fixtures.h"

#include "dsc_typed_vector_private.h"
#include "greatest.h"

#include <stdio.h>
#include <string.h>

static dsc_vector_int_t vec;

static void setup_vector(void* udata)
{
    (void)udata;
    typed_vector_test_reset_allocator();
    dsc_vector_int_init(&vec);
}

static void teardown_vector(void* udata)
{
    (void)udata;
    dsc_vector_int_free(&vec);
    typed_vector_test_reset_allocator();
}

static int push_ints(int count)
{
    for(int i = 0; i < count; ++i)
    {
        ASSERT(dsc_vector_int_push_back(&vec, i));
    }
    PASS();
}

#define ASSERT_SIZE(expected, actual) ASSERT_EQ_FMT((size_t)(expected), (size_t)(actual), "%zu")
#define ASSERT_ALIGNED(pointer, type) ASSERT_SIZE(0, (uintptr_t)(pointer) % alignof(type))

#define ASSERT_INT_AT(expected, index)                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        const int* value = dsc_vector_int_at_const(&vec, (index));                                                     \
        ASSERT(value != nullptr);                                                                                      \
        ASSERT_EQ_FMT((expected), *value, "%d");                                                                       \
    } while(0)

/* ---------- lifecycle ---------- */

TEST initialization_is_allocation_free(void)
{
    dsc_vector_int_t zero_initialized = {0};
    ASSERT(dsc_vector_int_empty(&zero_initialized));
    ASSERT_SIZE(0, dsc_vector_int_size(&zero_initialized));
    ASSERT_SIZE(0, dsc_vector_int_capacity(&zero_initialized));
    ASSERT_EQ(nullptr, dsc_vector_int_data(&zero_initialized));
    ASSERT(dsc_vector_int_empty(&vec));
    ASSERT_SIZE(0, dsc_vector_int_size(&vec));
    ASSERT_SIZE(0, dsc_vector_int_capacity(&vec));
    ASSERT_EQ(nullptr, dsc_vector_int_data(&vec));
    dsc_vector_int_free(&zero_initialized);
    PASS();
}

TEST clear_free_and_reuse(void)
{
    CHECK_CALL(push_ints(3));
    const size_t capacity = dsc_vector_int_capacity(&vec);
    int* allocation = vec.data;
    dsc_vector_int_clear(&vec);
    ASSERT(dsc_vector_int_empty(&vec));
    ASSERT_SIZE(capacity, dsc_vector_int_capacity(&vec));
    ASSERT_EQ(allocation, vec.data);
    ASSERT_EQ(nullptr, dsc_vector_int_data(&vec));
    CHECK_CALL(push_ints(2));
    ASSERT_EQ(allocation, vec.data);
    ASSERT_INT_AT(1, 1);

    dsc_vector_int_free(&vec);
    dsc_vector_int_free(&vec);
    ASSERT_SIZE(0, dsc_vector_int_size(&vec));
    ASSERT_SIZE(0, dsc_vector_int_capacity(&vec));
    ASSERT_EQ(nullptr, vec.data);
    CHECK_CALL(push_ints(2));
    ASSERT_INT_AT(1, 1);
    PASS();
}

TEST shrinking_an_empty_vector_releases_storage(void)
{
    ASSERT(dsc_vector_int_reserve(&vec, 12));
    ASSERT(vec.data != nullptr);
    ASSERT(dsc_vector_int_shrink_to_fit(&vec));
    ASSERT_EQ(nullptr, vec.data);
    ASSERT_SIZE(0, dsc_vector_int_capacity(&vec));
    CHECK_CALL(push_ints(1));
    dsc_vector_int_clear(&vec);
    ASSERT(dsc_vector_int_shrink_to_fit(&vec));
    ASSERT_EQ(nullptr, vec.data);
    ASSERT_SIZE(0, dsc_vector_int_capacity(&vec));
    PASS();
}

SUITE(typed_vector_lifecycle)
{
    SET_SETUP(setup_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    SHUFFLE_TESTS(12345, {
        RUN_TEST(initialization_is_allocation_free);
        RUN_TEST(clear_free_and_reuse);
        RUN_TEST(shrinking_an_empty_vector_releases_storage);
    });
}

/* ---------- capacity ---------- */

TEST custom_and_default_growth_are_independent(void)
{
    const size_t int_capacities[] = {4, 4, 4, 4, 6, 6, 10, 10, 10, 10, 16};
    for(size_t i = 0; i < sizeof(int_capacities) / sizeof(int_capacities[0]); ++i)
    {
        ASSERT(dsc_vector_int_push_back(&vec, (int)i));
        ASSERT_SIZE(int_capacities[i], dsc_vector_int_capacity(&vec));
    }

    dsc_vector_byte_t bytes = {0};
    const size_t byte_capacities[] = {1, 2, 3, 4, 6, 6, 9, 9, 9, 13};
    for(size_t i = 0; i < sizeof(byte_capacities) / sizeof(byte_capacities[0]); ++i)
    {
        ASSERT(dsc_vector_byte_push_back(&bytes, (unsigned char)i));
        ASSERT_SIZE(byte_capacities[i], dsc_vector_byte_capacity(&bytes));
    }
    dsc_vector_byte_free(&bytes);
    PASS();
}

TEST growth_near_one_always_makes_progress(void)
{
    dsc_vector_near_one_t values = {0};
    for(unsigned int i = 0; i < 16; ++i)
    {
        ASSERT(dsc_vector_near_one_push_back(&values, i));
        ASSERT_SIZE(i + 1, dsc_vector_near_one_capacity(&values));
        ASSERT_EQ_FMT(i, *dsc_vector_near_one_at(&values, i), "%u");
    }
    dsc_vector_near_one_free(&values);
    PASS();
}

TEST reserve_and_shrink_preserve_state_on_failure(void)
{
    struct test_record record = {.id = 7};
    dsc_vector_record_t records = {0};
    ASSERT(dsc_vector_record_push_back(&records, record));
    struct test_record* record_data = records.data;
    ASSERT_FALSE(dsc_vector_record_reserve(&records, SIZE_MAX));
    ASSERT_EQ(record_data, records.data);
    ASSERT_SIZE(1, dsc_vector_record_size(&records));
    ASSERT_SIZE(1, dsc_vector_record_capacity(&records));
    ASSERT_EQ_FMT(7, records.data[0].id, "%d");
    dsc_vector_record_free(&records);

    typed_vector_test_fail_next_allocation();
    ASSERT_FALSE(dsc_vector_int_reserve(&vec, 8));
    ASSERT_EQ(nullptr, vec.data);
    ASSERT_SIZE(0, vec.size);
    ASSERT_SIZE(0, vec.capacity);

    CHECK_CALL(push_ints(3));
    int* original_data = vec.data;
    const size_t original_capacity = vec.capacity;
    typed_vector_test_fail_next_allocation();
    ASSERT_FALSE(dsc_vector_int_reserve(&vec, 32));
    ASSERT_EQ(original_data, vec.data);
    ASSERT_SIZE(3, vec.size);
    ASSERT_SIZE(original_capacity, vec.capacity);
    ASSERT_INT_AT(0, 0);
    ASSERT_INT_AT(2, 2);

    ASSERT(dsc_vector_int_reserve(&vec, 32));
    original_data = vec.data;
    typed_vector_test_fail_next_allocation();
    ASSERT_FALSE(dsc_vector_int_shrink_to_fit(&vec));
    ASSERT_EQ(original_data, vec.data);
    ASSERT_SIZE(3, vec.size);
    ASSERT_SIZE(32, vec.capacity);
    ASSERT_INT_AT(1, 1);
    ASSERT(dsc_vector_int_shrink_to_fit(&vec));
    ASSERT_SIZE(3, vec.capacity);
    ASSERT_INT_AT(2, 2);
    PASS();
}

TEST reserve_and_shrink_noop_cases(void)
{
    ASSERT(dsc_vector_int_reserve(&vec, 0));
    ASSERT(dsc_vector_int_reserve(&vec, 3));
    ASSERT_EQ(nullptr, dsc_vector_int_data(&vec));
    int* allocation = vec.data;
    ASSERT(dsc_vector_int_reserve(&vec, 2));
    ASSERT(dsc_vector_int_reserve(&vec, 3));
    ASSERT_EQ(allocation, vec.data);
    CHECK_CALL(push_ints(3));
    ASSERT(dsc_vector_int_shrink_to_fit(&vec));
    ASSERT_EQ(allocation, vec.data);
    ASSERT_SIZE(3, vec.capacity);
    PASS();
}

SUITE(typed_vector_capacity)
{
    SET_SETUP(setup_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    SHUFFLE_TESTS(12345, {
        RUN_TEST(custom_and_default_growth_are_independent);
        RUN_TEST(growth_near_one_always_makes_progress);
        RUN_TEST(reserve_and_shrink_preserve_state_on_failure);
        RUN_TEST(reserve_and_shrink_noop_cases);
    });
}

/* ---------- modifiers ---------- */

TEST insert_erase_and_pop(void)
{
    ASSERT_FALSE(dsc_vector_int_pop_back(&vec));
    ASSERT_FALSE(dsc_vector_int_erase(&vec, 0));
    ASSERT(dsc_vector_int_insert(&vec, 0, 20));
    ASSERT(dsc_vector_int_insert(&vec, 0, 10));
    ASSERT(dsc_vector_int_insert(&vec, 2, 30));
    ASSERT(dsc_vector_int_insert(&vec, 1, 15));
    ASSERT_SIZE(4, vec.size);
    ASSERT_INT_AT(10, 0);
    ASSERT_INT_AT(15, 1);
    ASSERT_INT_AT(20, 2);
    ASSERT_INT_AT(30, 3);

    ASSERT(dsc_vector_int_erase(&vec, 0));
    ASSERT(dsc_vector_int_erase(&vec, 1));
    ASSERT_INT_AT(15, 0);
    ASSERT_INT_AT(30, 1);
    ASSERT(dsc_vector_int_pop_back(&vec));
    ASSERT(dsc_vector_int_pop_back(&vec));
    ASSERT_FALSE(dsc_vector_int_pop_back(&vec));
    ASSERT_FALSE(dsc_vector_int_erase(&vec, SIZE_MAX));
    ASSERT(dsc_vector_int_empty(&vec));
    PASS();
}

TEST values_from_the_vector_survive_growth_and_shifts(void)
{
    CHECK_CALL(push_ints(4));
    ASSERT(dsc_vector_int_push_back(&vec, *dsc_vector_int_at(&vec, 1)));
    ASSERT_SIZE(5, vec.size);
    ASSERT_INT_AT(1, 4);

    ASSERT(dsc_vector_int_insert(&vec, 0, *dsc_vector_int_at(&vec, 3)));
    ASSERT_INT_AT(3, 0);
    ASSERT_INT_AT(0, 1);
    ASSERT_INT_AT(3, 4);
    ASSERT(dsc_vector_int_shrink_to_fit(&vec));
    ASSERT(dsc_vector_int_insert(&vec, 2, *dsc_vector_int_at(&vec, 4)));
    const int expected[] = {3, 0, 3, 1, 2, 3, 1};
    ASSERT_SIZE(sizeof(expected) / sizeof(expected[0]), vec.size);
    for(size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i)
    {
        ASSERT_INT_AT(expected[i], i);
    }
    PASS();
}

TEST failed_insert_and_push_preserve_state(void)
{
    CHECK_CALL(push_ints(4));
    int* allocation = vec.data;
    typed_vector_test_fail_next_allocation();
    ASSERT_FALSE(dsc_vector_int_insert(&vec, SIZE_MAX, 42));
    ASSERT_FALSE(dsc_vector_int_push_back(&vec, 42));
    ASSERT_EQ(allocation, vec.data);
    ASSERT_SIZE(4, vec.size);
    ASSERT_SIZE(4, vec.capacity);

    typed_vector_test_fail_next_allocation();
    ASSERT_FALSE(dsc_vector_int_insert(&vec, 2, 42));
    ASSERT_EQ(allocation, vec.data);
    ASSERT_SIZE(4, vec.size);
    for(size_t i = 0; i < 4; ++i)
    {
        ASSERT_INT_AT((int)i, i);
    }
    ASSERT(dsc_vector_int_insert(&vec, 2, 42));
    ASSERT_INT_AT(42, 2);
    ASSERT_INT_AT(2, 3);
    PASS();
}

SUITE(typed_vector_modifiers)
{
    SET_SETUP(setup_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    SHUFFLE_TESTS(12345, {
        RUN_TEST(insert_erase_and_pop);
        RUN_TEST(values_from_the_vector_survive_growth_and_shifts);
        RUN_TEST(failed_insert_and_push_preserve_state);
    });
}

/* ---------- element access ---------- */

TEST accessors_and_modifiers_have_typed_signatures(void)
{
    static_assert(_Generic(dsc_vector_int_front(&vec), int*: 1, default: 0), "mutable front type");
    static_assert(_Generic(dsc_vector_int_back(&vec), int*: 1, default: 0), "mutable back type");
    static_assert(_Generic(dsc_vector_int_data(&vec), int*: 1, default: 0), "mutable data type");
    static_assert(_Generic(dsc_vector_int_at(&vec, 0), int*: 1, default: 0), "mutable element type");
    static_assert(_Generic(dsc_vector_int_front_const((dsc_vector_int_t const*)0), const int*: 1, default: 0),
                  "const front type");
    static_assert(_Generic(dsc_vector_int_back_const((dsc_vector_int_t const*)0), const int*: 1, default: 0),
                  "const back type");
    static_assert(_Generic(dsc_vector_int_data_const((dsc_vector_int_t const*)0), const int*: 1, default: 0),
                  "const data type");
    static_assert(_Generic(dsc_vector_int_at_const((dsc_vector_int_t const*)0, 0), const int*: 1, default: 0),
                  "const element type");
    static_assert(_Generic(dsc_vector_int_ptr_at((dsc_vector_int_ptr_t*)0, 0), int**: 1, default: 0),
                  "mutable pointer element type");
    static_assert(_Generic(dsc_vector_int_ptr_at_const((dsc_vector_int_ptr_t const*)0, 0), int* const*: 1, default: 0),
                  "const pointer element type");
    static_assert(_Generic(dsc_vector_const_int_ptr_at((dsc_vector_const_int_ptr_t*)0, 0), const int**: 1, default: 0),
                  "mutable const-pointee element type");
    static_assert(_Generic(dsc_vector_const_int_ptr_at_const((dsc_vector_const_int_ptr_t const*)0, 0),
                           const int* const*: 1, default: 0),
                  "const const-pointee element type");
    static_assert(_Generic(&dsc_vector_int_push_back, bool (*)(dsc_vector_int_t*, int): 1, default: 0),
                  "typed push signature");
    static_assert(_Generic(&dsc_vector_record_push_back,
                           bool (*)(dsc_vector_record_t*, struct test_record): 1, default: 0),
                  "typed struct push signature");
    PASS();
}

TEST accessors_on_empty_and_reserved_vectors(void)
{
    const dsc_vector_int_t* const_vec = &vec;
    for(int i = 0; i < 2; ++i)
    {
        ASSERT_EQ(nullptr, dsc_vector_int_front(&vec));
        ASSERT_EQ(nullptr, dsc_vector_int_back(&vec));
        ASSERT_EQ(nullptr, dsc_vector_int_data(&vec));
        ASSERT_EQ(nullptr, dsc_vector_int_at(&vec, 0));
        ASSERT_EQ(nullptr, dsc_vector_int_front_const(const_vec));
        ASSERT_EQ(nullptr, dsc_vector_int_back_const(const_vec));
        ASSERT_EQ(nullptr, dsc_vector_int_data_const(const_vec));
        ASSERT_EQ(nullptr, dsc_vector_int_at_const(const_vec, 0));
        ASSERT(dsc_vector_int_reserve(&vec, 8));
    }
    PASS();
}

TEST mutable_accessors_match_const_view(void)
{
    CHECK_CALL(push_ints(4));
    const dsc_vector_int_t* const_vec = &vec;
    ASSERT_EQ(dsc_vector_int_data(&vec), dsc_vector_int_front(&vec));
    ASSERT_EQ(dsc_vector_int_front(&vec), dsc_vector_int_front_const(const_vec));
    ASSERT_EQ(dsc_vector_int_back(&vec), dsc_vector_int_back_const(const_vec));
    ASSERT_EQ(dsc_vector_int_data(&vec), dsc_vector_int_data_const(const_vec));
    for(size_t i = 0; i < 4; ++i)
    {
        ASSERT_EQ(dsc_vector_int_data(&vec) + i, dsc_vector_int_at(&vec, i));
        ASSERT_EQ(dsc_vector_int_at(&vec, i), dsc_vector_int_at_const(const_vec, i));
    }
    ASSERT_EQ(nullptr, dsc_vector_int_at(&vec, 4));
    ASSERT_EQ(nullptr, dsc_vector_int_at_const(const_vec, SIZE_MAX));
    *dsc_vector_int_front(&vec) = 10;
    *dsc_vector_int_at(&vec, 1) = 20;
    dsc_vector_int_data(&vec)[2] = 30;
    *dsc_vector_int_back(&vec) = 40;
    ASSERT_EQ_FMT(10, *dsc_vector_int_front_const(const_vec), "%d");
    ASSERT_EQ_FMT(20, *dsc_vector_int_at_const(const_vec, 1), "%d");
    ASSERT_EQ_FMT(30, dsc_vector_int_data_const(const_vec)[2], "%d");
    ASSERT_EQ_FMT(40, *dsc_vector_int_back_const(const_vec), "%d");
    PASS();
}

SUITE(typed_vector_access)
{
    SET_SETUP(setup_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    SHUFFLE_TESTS(12345, {
        RUN_TEST(accessors_and_modifiers_have_typed_signatures);
        RUN_TEST(accessors_on_empty_and_reserved_vectors);
        RUN_TEST(mutable_accessors_match_const_view);
    });
}

/* ---------- element types ---------- */

TEST byte_and_double_elements(void)
{
    dsc_vector_byte_t bytes = {0};
    dsc_vector_double_t doubles = {0};
    for(unsigned int i = 0; i < 64; ++i)
    {
        ASSERT(dsc_vector_byte_push_back(&bytes, (unsigned char)i));
        ASSERT(dsc_vector_double_push_back(&doubles, (double)i + 0.5));
    }
    ASSERT(dsc_vector_byte_insert(&bytes, 17, 255));
    ASSERT(dsc_vector_byte_erase(&bytes, 17));
    ASSERT(dsc_vector_double_insert(&doubles, 12, *dsc_vector_double_at(&doubles, 40)));
    ASSERT(dsc_vector_double_erase(&doubles, 12));
    ASSERT(dsc_vector_byte_reserve(&bytes, 128));
    ASSERT(dsc_vector_double_reserve(&doubles, 128));
    ASSERT(dsc_vector_byte_shrink_to_fit(&bytes));
    ASSERT(dsc_vector_double_shrink_to_fit(&doubles));
    ASSERT_SIZE(64, bytes.capacity);
    ASSERT_SIZE(64, doubles.capacity);
    for(size_t i = 0; i < 64; ++i)
    {
        ASSERT_EQ_FMT((unsigned int)i, (unsigned int)*dsc_vector_byte_at(&bytes, i), "%u");
        ASSERT(*dsc_vector_double_at(&doubles, i) == (double)i + 0.5);
        ASSERT_ALIGNED(dsc_vector_byte_at(&bytes, i), unsigned char);
        ASSERT_ALIGNED(dsc_vector_double_at(&doubles, i), double);
    }
    dsc_vector_byte_free(&bytes);
    dsc_vector_double_free(&doubles);
    PASS();
}

TEST pointer_elements_are_shallow_copies(void)
{
    int first = 5;
    int second = 9;
    dsc_vector_int_ptr_t values = {0};
    dsc_vector_const_int_ptr_t const_values = {0};
    ASSERT(dsc_vector_int_ptr_push_back(&values, &first));
    ASSERT(dsc_vector_int_ptr_push_back(&values, &second));
    ASSERT(dsc_vector_int_ptr_push_back(&values, nullptr));
    ASSERT(dsc_vector_const_int_ptr_push_back(&const_values, &first));
    int* const* const_first = dsc_vector_int_ptr_front_const(&values);
    ASSERT_EQ(&first, *const_first);
    ASSERT_EQ(&first, *dsc_vector_const_int_ptr_front_const(&const_values));
    **dsc_vector_int_ptr_at(&values, 1) = 12;
    ASSERT_EQ_FMT(12, second, "%d");
    ASSERT_EQ(nullptr, *dsc_vector_int_ptr_back_const(&values));
    ASSERT(dsc_vector_int_ptr_erase(&values, 0));
    ASSERT_EQ(&second, *dsc_vector_int_ptr_front(&values));
    dsc_vector_int_ptr_free(&values);
    dsc_vector_const_int_ptr_free(&const_values);
    ASSERT_EQ_FMT(5, first, "%d");
    ASSERT_EQ_FMT(12, second, "%d");
    PASS();
}

TEST complex_struct_elements_survive_reallocation(void)
{
    int owner = 42;
    dsc_vector_record_t records = {0};
    for(int i = 0; i < 32; ++i)
    {
        struct test_record record = {
            .id = i, .position = {(double)i + 0.5, -(double)i}, .samples = {(double)i, 1.5, 2.5}, .owner = &owner};
        snprintf(record.name, sizeof(record.name), "record-%d", i);
        ASSERT(dsc_vector_record_push_back(&records, record));
    }
    ASSERT(dsc_vector_record_shrink_to_fit(&records));
    ASSERT(dsc_vector_record_push_back(&records, *dsc_vector_record_at(&records, 0)));
    ASSERT(dsc_vector_record_insert(&records, 0, *dsc_vector_record_at(&records, 10)));
    ASSERT_EQ_FMT(10, dsc_vector_record_front(&records)->id, "%d");
    ASSERT(dsc_vector_record_erase(&records, 0));
    ASSERT(dsc_vector_record_reserve(&records, 100));
    ASSERT(dsc_vector_record_shrink_to_fit(&records));
    ASSERT_SIZE(33, dsc_vector_record_size(&records));
    for(int i = 0; i < 32; ++i)
    {
        const struct test_record* record = dsc_vector_record_at_const(&records, (size_t)i);
        char expected_name[24];
        snprintf(expected_name, sizeof(expected_name), "record-%d", i);
        ASSERT_EQ_FMT(i, record->id, "%d");
        ASSERT_STR_EQ(expected_name, record->name);
        ASSERT(record->position.x == (double)i + 0.5);
        ASSERT(record->position.y == -(double)i);
        ASSERT(record->samples[0] == (double)i);
        ASSERT(record->samples[1] == 1.5);
        ASSERT_EQ(&owner, record->owner);
    }
    ASSERT_EQ_FMT(0, dsc_vector_record_back(&records)->id, "%d");
    ASSERT_EQ(&owner, dsc_vector_record_back(&records)->owner);
    dsc_vector_record_free(&records);
    ASSERT_EQ_FMT(42, owner, "%d");
    PASS();
}

SUITE(typed_vector_element_types)
{
    SET_SETUP(setup_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    SHUFFLE_TESTS(12345, {
        RUN_TEST(byte_and_double_elements);
        RUN_TEST(pointer_elements_are_shallow_copies);
        RUN_TEST(complex_struct_elements_survive_reallocation);
    });
}

/* ---------- alignment ---------- */

TEST fundamental_alignment_is_preserved(void)
{
    dsc_vector_native_aligned_t values = {0};
    for(unsigned int i = 0; i < 32; ++i)
    {
        struct test_native_aligned value = {.bytes = {(unsigned char)i}, .id = i};
        ASSERT(dsc_vector_native_aligned_push_back(&values, value));
        ASSERT_ALIGNED(dsc_vector_native_aligned_back(&values), struct test_native_aligned);
    }
    ASSERT(dsc_vector_native_aligned_reserve(&values, 80));
    ASSERT_ALIGNED(values.data, struct test_native_aligned);
    ASSERT(dsc_vector_native_aligned_shrink_to_fit(&values));
    for(size_t i = 0; i < 32; ++i)
    {
        const struct test_native_aligned* value = dsc_vector_native_aligned_at_const(&values, i);
        ASSERT_ALIGNED(value, struct test_native_aligned);
        ASSERT_EQ_FMT((unsigned int)i, value->id, "%u");
        ASSERT_EQ_FMT((unsigned int)i, (unsigned int)value->bytes[0], "%u");
    }
    dsc_vector_native_aligned_free(&values);
    PASS();
}

TEST extended_alignment_survives_growth_insert_and_shrink(void)
{
    static_assert(alignof(struct test_aligned_64) >= 64, "64-byte alignment required");
    static_assert(alignof(struct test_aligned_128) >= 128, "128-byte alignment required");
    dsc_vector_aligned_64_t values64 = {0};
    dsc_vector_aligned_128_t values128 = {0};
    for(unsigned int i = 0; i < 40; ++i)
    {
        struct test_aligned_64 value64 = {.id = i, .samples = {(double)i + 0.5, 2.5, 3.5}};
        struct test_aligned_128 value128 = {.id = i, .bytes = {(unsigned char)i}};
        ASSERT(dsc_vector_aligned_64_push_back(&values64, value64));
        ASSERT(dsc_vector_aligned_128_push_back(&values128, value128));
        ASSERT_ALIGNED(dsc_vector_aligned_64_back(&values64), struct test_aligned_64);
        ASSERT_ALIGNED(dsc_vector_aligned_128_back(&values128), struct test_aligned_128);
    }
    if(alignof(struct test_aligned_64) > alignof(max_align_t))
    {
        ASSERT(typed_vector_test_aligned_allocation_count() > 0);
    }

    ASSERT(dsc_vector_aligned_64_reserve(&values64, 100));
    ASSERT(dsc_vector_aligned_128_reserve(&values128, 100));
    ASSERT_ALIGNED(values64.data, struct test_aligned_64);
    ASSERT_ALIGNED(values128.data, struct test_aligned_128);
    ASSERT(dsc_vector_aligned_64_shrink_to_fit(&values64));
    ASSERT(dsc_vector_aligned_128_shrink_to_fit(&values128));
    ASSERT_ALIGNED(values64.data, struct test_aligned_64);
    ASSERT_ALIGNED(values128.data, struct test_aligned_128);
    ASSERT(dsc_vector_aligned_64_insert(&values64, 0, *dsc_vector_aligned_64_at(&values64, 39)));
    ASSERT(dsc_vector_aligned_128_insert(&values128, 0, *dsc_vector_aligned_128_at(&values128, 39)));
    ASSERT_EQ_FMT(39U, (unsigned int)dsc_vector_aligned_64_front(&values64)->id, "%u");
    ASSERT_EQ_FMT(39U, (unsigned int)dsc_vector_aligned_128_front(&values128)->id, "%u");
    ASSERT(dsc_vector_aligned_64_erase(&values64, 0));
    ASSERT(dsc_vector_aligned_128_erase(&values128, 0));
    for(size_t i = 0; i < 40; ++i)
    {
        const struct test_aligned_64* value64 = dsc_vector_aligned_64_at_const(&values64, i);
        const struct test_aligned_128* value128 = dsc_vector_aligned_128_at_const(&values128, i);
        ASSERT_ALIGNED(value64, struct test_aligned_64);
        ASSERT_ALIGNED(value128, struct test_aligned_128);
        ASSERT_EQ_FMT((unsigned int)i, (unsigned int)value64->id, "%u");
        ASSERT(value64->samples[0] == (double)i + 0.5);
        ASSERT_EQ_FMT((unsigned int)i, (unsigned int)value128->id, "%u");
        ASSERT_EQ_FMT((unsigned int)i, (unsigned int)value128->bytes[0], "%u");
    }
    dsc_vector_aligned_64_free(&values64);
    dsc_vector_aligned_128_free(&values128);
    PASS();
}

TEST aligned_allocation_failures_preserve_state(void)
{
    dsc_vector_aligned_128_t values = {0};
    struct test_aligned_128 value = {.id = 123, .bytes = {42}};
    typed_vector_test_fail_next_allocation();
    ASSERT_FALSE(dsc_vector_aligned_128_push_back(&values, value));
    ASSERT_EQ(nullptr, values.data);
    ASSERT_SIZE(0, values.size);
    ASSERT_SIZE(0, values.capacity);
    ASSERT(dsc_vector_aligned_128_push_back(&values, value));
    struct test_aligned_128* original_data = values.data;
    typed_vector_test_fail_next_allocation();
    ASSERT_FALSE(dsc_vector_aligned_128_reserve(&values, 8));
    ASSERT_EQ(original_data, values.data);
    ASSERT_SIZE(1, values.size);
    ASSERT_SIZE(1, values.capacity);
    ASSERT_EQ_FMT(123U, (unsigned int)values.data[0].id, "%u");

    ASSERT(dsc_vector_aligned_128_reserve(&values, 8));
    original_data = values.data;
    typed_vector_test_fail_next_allocation();
    ASSERT_FALSE(dsc_vector_aligned_128_shrink_to_fit(&values));
    ASSERT_EQ(original_data, values.data);
    ASSERT_SIZE(1, values.size);
    ASSERT_SIZE(8, values.capacity);
    ASSERT_EQ_FMT(42U, (unsigned int)values.data[0].bytes[0], "%u");
    ASSERT(dsc_vector_aligned_128_shrink_to_fit(&values));
    ASSERT_SIZE(1, values.capacity);
    ASSERT_ALIGNED(values.data, struct test_aligned_128);

    dsc_vector_aligned_128_clear(&values);
    original_data = values.data;
    typed_vector_test_fail_next_allocation();
    ASSERT_FALSE(dsc_vector_aligned_128_reserve(&values, 8));
    ASSERT_EQ(original_data, values.data);
    ASSERT_SIZE(0, values.size);
    ASSERT_SIZE(1, values.capacity);
    ASSERT(dsc_vector_aligned_128_reserve(&values, 8));
    ASSERT_ALIGNED(values.data, struct test_aligned_128);
    ASSERT_EQ(nullptr, dsc_vector_aligned_128_data(&values));
    ASSERT(dsc_vector_aligned_128_shrink_to_fit(&values));
    ASSERT_EQ(nullptr, values.data);
    ASSERT_SIZE(0, values.capacity);
    dsc_vector_aligned_128_free(&values);
    PASS();
}

SUITE(typed_vector_alignment)
{
    SET_SETUP(setup_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    SHUFFLE_TESTS(12345, {
        RUN_TEST(fundamental_alignment_is_preserved);
        RUN_TEST(extended_alignment_survives_growth_insert_and_shrink);
        RUN_TEST(aligned_allocation_failures_preserve_state);
    });
}

/* ---------- growth arithmetic ---------- */

TEST growth_arithmetic_handles_large_values(void)
{
    const size_t half = SIZE_MAX / 2;
    ASSERT_SIZE(6, dsc_typed_vector_private_scaled_capacity(4, 5, 3, SIZE_MAX));
    ASSERT_SIZE(10, dsc_typed_vector_private_scaled_capacity(8, 3, 2, 10));
    ASSERT_SIZE(half + half / 2, dsc_typed_vector_private_scaled_capacity(half, 3, 2, SIZE_MAX));
    ASSERT_SIZE(half + half / 3, dsc_typed_vector_private_scaled_capacity(half, 4, 3, SIZE_MAX));
    ASSERT_SIZE(SIZE_MAX - 2,
                dsc_typed_vector_private_scaled_capacity(half, SIZE_MAX, half + 1, SIZE_MAX));
    ASSERT_SIZE(23, dsc_typed_vector_private_scaled_capacity(23, SIZE_MAX, SIZE_MAX - 1, SIZE_MAX));
    ASSERT_SIZE(SIZE_MAX, dsc_typed_vector_private_scaled_capacity(SIZE_MAX - 1, SIZE_MAX, SIZE_MAX - 1, SIZE_MAX));
    PASS();
}

#if defined(__SIZEOF_INT128__) && defined(__SIZEOF_SIZE_T__) && __SIZEOF_SIZE_T__ <= 8
__extension__ typedef unsigned __int128 test_wide_uint;

static uint64_t next_random(uint64_t* state)
{
    *state = *state * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
    return *state;
}

TEST growth_arithmetic_matches_wide_reference(void)
{
    uint64_t state = 12345;
    for(size_t i = 0; i < 10000; ++i)
    {
        const size_t capacity = (size_t)next_random(&state);
        const size_t denominator = ((size_t)next_random(&state) >> 1) + 1;
        const size_t numerator = denominator + 1 + (size_t)next_random(&state) % (SIZE_MAX - denominator);
        const size_t limit = (size_t)next_random(&state);
        const test_wide_uint scaled = (test_wide_uint)capacity * numerator / denominator;
        const size_t expected = scaled > limit ? limit : (size_t)scaled;
        ASSERT_SIZE(expected, dsc_typed_vector_private_scaled_capacity(capacity, numerator, denominator, limit));
    }
    PASS();
}
#else
TEST growth_arithmetic_matches_wide_reference(void)
{
    SKIP();
}
#endif

SUITE(typed_vector_arithmetic)
{
    SET_SETUP(setup_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    SHUFFLE_TESTS(12345, {
        RUN_TEST(growth_arithmetic_handles_large_values);
        RUN_TEST(growth_arithmetic_matches_wide_reference);
    });
}

GREATEST_MAIN_DEFS();

int main(int argc, char** argv)
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(typed_vector_lifecycle);
    RUN_SUITE(typed_vector_capacity);
    RUN_SUITE(typed_vector_modifiers);
    RUN_SUITE(typed_vector_access);
    RUN_SUITE(typed_vector_element_types);
    RUN_SUITE(typed_vector_alignment);
    RUN_SUITE(typed_vector_arithmetic);
    GREATEST_MAIN_END();
}
