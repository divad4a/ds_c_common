#include "greatest.h"
#include "vector.h"

#include <stdint.h>

static struct DS_COMMON_Vector vec;

static void setup_int_vector(void* udata)
{
    (void)udata;
    dsc_vector_init(&vec, sizeof(int));
}

static void teardown_vector(void* udata)
{
    (void)udata;
    dsc_vector_free(&vec);
}

static int push_ints(int count)
{
    for(int i = 0; i < count; ++i)
    {
        ASSERT(dsc_vector_push_back(&vec, &i));
    }
    PASS();
}

#define ASSERT_SIZE(expected, actual) ASSERT_EQ_FMT((size_t)(expected), (actual), "%zu")

#define ASSERT_INT_AT(expected, index)                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        const int* _p = dsc_vector_at(&vec, (index));                                                                  \
        ASSERT(_p != nullptr);                                                                                         \
        ASSERT_EQ_FMT((expected), *_p, "%d");                                                                          \
    } while(0)

/* ---------- lifecycle ---------- */

TEST init_creates_empty_vector(void)
{
    ASSERT_EQ(nullptr, dsc_vector_data(&vec));
    ASSERT_SIZE(0, dsc_vector_size(&vec));
    ASSERT_SIZE(0, dsc_vector_capacity(&vec));
    ASSERT_SIZE(sizeof(int), dsc_vector_element_size(&vec));
    ASSERT(dsc_vector_empty(&vec));
    PASS();
}

TEST free_resets_vector(void)
{
    CHECK_CALL(push_ints(5));
    dsc_vector_free(&vec);
    ASSERT_EQ(nullptr, dsc_vector_data(&vec));
    ASSERT_SIZE(0, dsc_vector_size(&vec));
    ASSERT_SIZE(0, dsc_vector_capacity(&vec));
    ASSERT_SIZE(sizeof(int), dsc_vector_element_size(&vec));
    PASS();
}

TEST clear_keeps_capacity(void)
{
    CHECK_CALL(push_ints(5));
    size_t capacity = dsc_vector_capacity(&vec);
    dsc_vector_clear(&vec);
    ASSERT(dsc_vector_empty(&vec));
    ASSERT_SIZE(capacity, dsc_vector_capacity(&vec));
    CHECK_CALL(push_ints(2));
    ASSERT_SIZE(2, dsc_vector_size(&vec));
    ASSERT_SIZE(capacity, dsc_vector_capacity(&vec));
    ASSERT_INT_AT(0, 0);
    ASSERT_INT_AT(1, 1);
    PASS();
}

TEST free_is_repeatable_and_vector_is_reusable(void)
{
    dsc_vector_free(&vec);
    dsc_vector_free(&vec);
    CHECK_CALL(push_ints(3));
    dsc_vector_free(&vec);
    dsc_vector_free(&vec);
    CHECK_CALL(push_ints(2));
    ASSERT_SIZE(2, dsc_vector_size(&vec));
    ASSERT_INT_AT(0, 0);
    ASSERT_INT_AT(1, 1);
    PASS();
}

SUITE(vector_lifecycle)
{
    SET_SETUP(setup_int_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    SHUFFLE_TESTS(12345, {
        RUN_TEST(init_creates_empty_vector);
        RUN_TEST(free_resets_vector);
        RUN_TEST(clear_keeps_capacity);
        RUN_TEST(free_is_repeatable_and_vector_is_reusable);
    });
}

/* ---------- capacity ---------- */

TEST reserve_increases_capacity(void)
{
    ASSERT(dsc_vector_reserve(&vec, 16));
    ASSERT_SIZE(16, dsc_vector_capacity(&vec));
    ASSERT_SIZE(0, dsc_vector_size(&vec));
    PASS();
}

TEST reserve_smaller_does_not_shrink(void)
{
    ASSERT(dsc_vector_reserve(&vec, 16));
    ASSERT(dsc_vector_reserve(&vec, 4));
    ASSERT_SIZE(16, dsc_vector_capacity(&vec));
    PASS();
}

TEST push_back_grows_capacity(void)
{
    size_t capacity = 0;
    for(int i = 0; i < 100; ++i)
    {
        ASSERT(dsc_vector_push_back(&vec, &i));
        ASSERT_SIZE(i + 1, dsc_vector_size(&vec));
        ASSERT(dsc_vector_capacity(&vec) >= (size_t)i + 1);
        ASSERT(dsc_vector_capacity(&vec) >= capacity);
        capacity = dsc_vector_capacity(&vec);
    }
    ASSERT_SIZE(100, dsc_vector_size(&vec));
    ASSERT(dsc_vector_capacity(&vec) >= 100);
    for(int i = 0; i < 100; ++i)
    {
        ASSERT_INT_AT(i, (size_t)i);
    }
    PASS();
}

TEST shrink_to_fit_matches_size(void)
{
    ASSERT(dsc_vector_reserve(&vec, 32));
    CHECK_CALL(push_ints(3));
    ASSERT(dsc_vector_shrink_to_fit(&vec));
    ASSERT_SIZE(3, dsc_vector_capacity(&vec));
    ASSERT_INT_AT(0, 0);
    ASSERT_INT_AT(1, 1);
    ASSERT_INT_AT(2, 2);
    PASS();
}

TEST reserve_preserves_existing_elements(void)
{
    CHECK_CALL(push_ints(5));
    ASSERT(dsc_vector_reserve(&vec, 64));
    ASSERT_SIZE(5, dsc_vector_size(&vec));
    ASSERT_SIZE(64, dsc_vector_capacity(&vec));
    void* data = dsc_vector_data(&vec);
    ASSERT(dsc_vector_reserve(&vec, 64));
    ASSERT(dsc_vector_reserve(&vec, 1));
    ASSERT_SIZE(64, dsc_vector_capacity(&vec));
    ASSERT_EQ(data, dsc_vector_data(&vec));
    for(int i = 0; i < 5; ++i)
    {
        ASSERT_INT_AT(i, (size_t)i);
    }
    PASS();
}

TEST shrink_noop_cases(void)
{
    ASSERT(dsc_vector_shrink_to_fit(&vec));
    ASSERT_SIZE(0, dsc_vector_capacity(&vec));
    ASSERT(dsc_vector_reserve(&vec, 3));
    CHECK_CALL(push_ints(3));
    void* data = dsc_vector_data(&vec);
    ASSERT(dsc_vector_shrink_to_fit(&vec));
    ASSERT_SIZE(3, dsc_vector_capacity(&vec));
    ASSERT_SIZE(3, dsc_vector_size(&vec));
    ASSERT_EQ(data, dsc_vector_data(&vec));
    ASSERT_INT_AT(2, 2);
    PASS();
}

TEST reserve_overflow_on_empty_preserves_state(void)
{
    ASSERT_FALSE(dsc_vector_reserve(&vec, SIZE_MAX));
    ASSERT_SIZE(0, dsc_vector_size(&vec));
    ASSERT_SIZE(0, dsc_vector_capacity(&vec));
    ASSERT_EQ(nullptr, dsc_vector_data(&vec));
    CHECK_CALL(push_ints(2));
    PASS();
}

TEST reserve_overflow_preserves_existing_elements(void)
{
    CHECK_CALL(push_ints(3));
    size_t capacity = dsc_vector_capacity(&vec);
    void* data = dsc_vector_data(&vec);
    ASSERT_FALSE(dsc_vector_reserve(&vec, SIZE_MAX));
    ASSERT_SIZE(3, dsc_vector_size(&vec));
    ASSERT_SIZE(capacity, dsc_vector_capacity(&vec));
    ASSERT_EQ(data, dsc_vector_data(&vec));
    for(int i = 0; i < 3; ++i)
    {
        ASSERT_INT_AT(i, (size_t)i);
    }
    PASS();
}

SUITE(vector_capacity)
{
    SET_SETUP(setup_int_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    SHUFFLE_TESTS(12345, {
        RUN_TEST(reserve_increases_capacity);
        RUN_TEST(reserve_smaller_does_not_shrink);
        RUN_TEST(push_back_grows_capacity);
        RUN_TEST(shrink_to_fit_matches_size);
        RUN_TEST(reserve_preserves_existing_elements);
        RUN_TEST(shrink_noop_cases);
        RUN_TEST(reserve_overflow_on_empty_preserves_state);
        RUN_TEST(reserve_overflow_preserves_existing_elements);
    });
}

/* ---------- modifiers ---------- */

TEST push_and_pop_back(void)
{
    CHECK_CALL(push_ints(3));
    ASSERT(dsc_vector_pop_back(&vec));
    ASSERT_SIZE(2, dsc_vector_size(&vec));
    ASSERT_INT_AT(1, 1);
    ASSERT(dsc_vector_pop_back(&vec));
    ASSERT(dsc_vector_pop_back(&vec));
    ASSERT(dsc_vector_empty(&vec));
    ASSERT_FALSE(dsc_vector_pop_back(&vec));
    CHECK_CALL(push_ints(2));
    ASSERT_INT_AT(1, 1);
    PASS();
}

TEST pop_back_on_empty_fails(void)
{
    ASSERT_FALSE(dsc_vector_pop_back(&vec));
    ASSERT_SIZE(0, dsc_vector_size(&vec));
    ASSERT_SIZE(0, dsc_vector_capacity(&vec));
    ASSERT_EQ(nullptr, dsc_vector_data(&vec));
    PASS();
}

TEST insert_at_front_middle_and_end(void)
{
    int values[] = {10, 20, 30};
    ASSERT(dsc_vector_insert(&vec, 0, &values[1])); /* [20] */
    ASSERT(dsc_vector_insert(&vec, 0, &values[0])); /* [10, 20] */
    ASSERT(dsc_vector_insert(&vec, 2, &values[2])); /* [10, 20, 30] */
    int middle = 15;
    ASSERT(dsc_vector_insert(&vec, 1, &middle)); /* [10, 15, 20, 30] */

    ASSERT_SIZE(4, dsc_vector_size(&vec));
    ASSERT_INT_AT(10, 0);
    ASSERT_INT_AT(15, 1);
    ASSERT_INT_AT(20, 2);
    ASSERT_INT_AT(30, 3);
    PASS();
}

TEST insert_out_of_range_fails(void)
{
    int value = 1;
    ASSERT_FALSE(dsc_vector_insert(&vec, 1, &value));
    ASSERT_SIZE(0, dsc_vector_size(&vec));
    ASSERT_SIZE(0, dsc_vector_capacity(&vec));
    PASS();
}

TEST insert_out_of_range_preserves_full_vector(void)
{
    ASSERT(dsc_vector_reserve(&vec, 2));
    CHECK_CALL(push_ints(2));
    void* data = dsc_vector_data(&vec);
    int value = 42;
    ASSERT_FALSE(dsc_vector_insert(&vec, 3, &value));
    ASSERT_SIZE(2, dsc_vector_size(&vec));
    ASSERT_SIZE(2, dsc_vector_capacity(&vec));
    ASSERT_EQ(data, dsc_vector_data(&vec));
    ASSERT_INT_AT(0, 0);
    ASSERT_INT_AT(1, 1);
    PASS();
}

TEST erase_removes_and_shifts(void)
{
    CHECK_CALL(push_ints(5));          /* [0, 1, 2, 3, 4] */
    ASSERT(dsc_vector_erase(&vec, 0)); /* [1, 2, 3, 4] */
    ASSERT(dsc_vector_erase(&vec, 1)); /* [1, 3, 4] */
    ASSERT(dsc_vector_erase(&vec, 2)); /* [1, 3] */

    ASSERT_SIZE(2, dsc_vector_size(&vec));
    ASSERT_INT_AT(1, 0);
    ASSERT_INT_AT(3, 1);
    PASS();
}

TEST erase_out_of_range_fails(void)
{
    CHECK_CALL(push_ints(2));
    size_t capacity = dsc_vector_capacity(&vec);
    void* data = dsc_vector_data(&vec);
    ASSERT_FALSE(dsc_vector_erase(&vec, 2));
    ASSERT_FALSE(dsc_vector_erase(&vec, SIZE_MAX));
    ASSERT_SIZE(2, dsc_vector_size(&vec));
    ASSERT_SIZE(capacity, dsc_vector_capacity(&vec));
    ASSERT_EQ(data, dsc_vector_data(&vec));
    ASSERT_INT_AT(0, 0);
    ASSERT_INT_AT(1, 1);
    PASS();
}

TEST erase_last_and_only_element(void)
{
    ASSERT_FALSE(dsc_vector_erase(&vec, 0));
    CHECK_CALL(push_ints(2));
    size_t capacity = dsc_vector_capacity(&vec);
    ASSERT(dsc_vector_erase(&vec, 1));
    ASSERT_SIZE(1, dsc_vector_size(&vec));
    ASSERT_INT_AT(0, 0);
    ASSERT(dsc_vector_erase(&vec, 0));
    ASSERT(dsc_vector_empty(&vec));
    ASSERT_SIZE(capacity, dsc_vector_capacity(&vec));
    ASSERT_FALSE(dsc_vector_erase(&vec, 0));
    PASS();
}

TEST insert_into_full_vector(void)
{
    ASSERT(dsc_vector_reserve(&vec, 2));
    CHECK_CALL(push_ints(2));
    int value = 42;
    ASSERT(dsc_vector_insert(&vec, 1, &value));
    ASSERT_SIZE(3, dsc_vector_size(&vec));
    ASSERT(dsc_vector_capacity(&vec) >= 3);
    ASSERT_INT_AT(0, 0);
    ASSERT_INT_AT(42, 1);
    ASSERT_INT_AT(1, 2);
    ASSERT(dsc_vector_shrink_to_fit(&vec));
    value = 99;
    ASSERT(dsc_vector_insert(&vec, 3, &value));
    ASSERT_SIZE(4, dsc_vector_size(&vec));
    ASSERT_INT_AT(0, 0);
    ASSERT_INT_AT(42, 1);
    ASSERT_INT_AT(1, 2);
    ASSERT_INT_AT(99, 3);
    PASS();
}

SUITE(vector_modifiers)
{
    SET_SETUP(setup_int_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    SHUFFLE_TESTS(12345, {
        RUN_TEST(push_and_pop_back);
        RUN_TEST(pop_back_on_empty_fails);
        RUN_TEST(insert_at_front_middle_and_end);
        RUN_TEST(insert_out_of_range_fails);
        RUN_TEST(insert_out_of_range_preserves_full_vector);
        RUN_TEST(erase_removes_and_shifts);
        RUN_TEST(erase_out_of_range_fails);
        RUN_TEST(erase_last_and_only_element);
        RUN_TEST(insert_into_full_vector);
    });
}

/* ---------- element access ---------- */

TEST accessors_preserve_pointer_types(void)
{
    const struct DS_COMMON_Vector* cvec = &vec;
    static_assert(_Generic(dsc_vector_front(&vec), void*: 1, default: 0), "mutable front must return void*");
    static_assert(_Generic(dsc_vector_back(&vec), void*: 1, default: 0), "mutable back must return void*");
    static_assert(_Generic(dsc_vector_data(&vec), void*: 1, default: 0), "mutable data must return void*");
    static_assert(_Generic(dsc_vector_at(&vec, 0), void*: 1, default: 0), "mutable at must return void*");
    static_assert(_Generic(dsc_vector_front(cvec), const void*: 1, default: 0), "const front must return const void*");
    static_assert(_Generic(dsc_vector_back(cvec), const void*: 1, default: 0), "const back must return const void*");
    static_assert(_Generic(dsc_vector_data(cvec), const void*: 1, default: 0), "const data must return const void*");
    static_assert(_Generic(dsc_vector_at(cvec, 0), const void*: 1, default: 0), "const at must return const void*");
    PASS();
}

TEST accessors_on_empty_return_null(void)
{
    const struct DS_COMMON_Vector* cvec = &vec;
    for(int state = 0; state < 3; ++state)
    {
        ASSERT(dsc_vector_empty(&vec));
        ASSERT_EQ(nullptr, dsc_vector_front(&vec));
        ASSERT_EQ(nullptr, dsc_vector_back(&vec));
        ASSERT_EQ(nullptr, dsc_vector_data(&vec));
        ASSERT_EQ(nullptr, dsc_vector_at(&vec, 0));
        ASSERT_EQ(nullptr, dsc_vector_front(cvec));
        ASSERT_EQ(nullptr, dsc_vector_back(cvec));
        ASSERT_EQ(nullptr, dsc_vector_data(cvec));
        ASSERT_EQ(nullptr, dsc_vector_at(cvec, 0));
        if(state == 0)
        {
            ASSERT(dsc_vector_reserve(&vec, 8));
        }
        else if(state == 1)
        {
            CHECK_CALL(push_ints(3));
            dsc_vector_clear(&vec);
        }
    }
    PASS();
}

TEST front_back_data_at(void)
{
    CHECK_CALL(push_ints(4));
    int* front = dsc_vector_front(&vec);
    int* back = dsc_vector_back(&vec);
    ASSERT(front != nullptr);
    ASSERT(back != nullptr);
    ASSERT_EQ_FMT(0, *front, "%d");
    ASSERT_EQ_FMT(3, *back, "%d");
    ASSERT_EQ(dsc_vector_front(&vec), dsc_vector_data(&vec));
    ASSERT_EQ(nullptr, dsc_vector_at(&vec, 4));
    ASSERT_EQ(nullptr, dsc_vector_at(&vec, SIZE_MAX));
    ASSERT_EQ(dsc_vector_back(&vec), dsc_vector_at(&vec, 3));
    const int* data = dsc_vector_data(&vec);
    for(size_t i = 0; i < 4; ++i)
    {
        ASSERT_EQ(data + i, dsc_vector_at(&vec, i));
        ASSERT_EQ_FMT((int)i, data[i], "%d");
    }
    PASS();
}

TEST at_allows_modification(void)
{
    CHECK_CALL(push_ints(3));
    int* element = dsc_vector_at(&vec, 1);
    ASSERT(element != nullptr);
    *element = 42;
    ASSERT_INT_AT(42, 1);
    PASS();
}

TEST mutable_accessors_update_const_view(void)
{
    CHECK_CALL(push_ints(4));
    const struct DS_COMMON_Vector* cvec = &vec;
    const int* const_front = dsc_vector_front(cvec);
    const int* const_back = dsc_vector_back(cvec);
    const int* const_data = dsc_vector_data(cvec);
    const int* const_element = dsc_vector_at(cvec, 1);
    int* front = dsc_vector_front(&vec);
    int* back = dsc_vector_back(&vec);
    int* data = dsc_vector_data(&vec);
    int* element = dsc_vector_at(&vec, 1);
    ASSERT(front != nullptr);
    ASSERT(back != nullptr);
    ASSERT(data != nullptr);
    ASSERT(element != nullptr);
    ASSERT_EQ(front, const_front);
    ASSERT_EQ(back, const_back);
    ASSERT_EQ(data, const_data);
    ASSERT_EQ(element, const_element);

    *front = 10;
    *back = 30;
    data[2] = 20;
    *element = 42;
    ASSERT_EQ_FMT(10, *const_front, "%d");
    ASSERT_EQ_FMT(30, *const_back, "%d");
    ASSERT_EQ_FMT(20, const_data[2], "%d");
    ASSERT_EQ_FMT(42, *const_element, "%d");
    ASSERT_SIZE(4, dsc_vector_size(cvec));
    PASS();
}

TEST const_accessors(void)
{
    CHECK_CALL(push_ints(3));
    const struct DS_COMMON_Vector* cvec = &vec;
    const int* front = dsc_vector_front(cvec);
    const int* back = dsc_vector_back(cvec);
    const int* element = dsc_vector_at(cvec, 1);
    const int* data = dsc_vector_data(cvec);
    ASSERT(front != nullptr);
    ASSERT(back != nullptr);
    ASSERT(element != nullptr);
    ASSERT(data != nullptr);
    ASSERT_EQ_FMT(0, *front, "%d");
    ASSERT_EQ_FMT(2, *back, "%d");
    ASSERT_EQ_FMT(1, *element, "%d");
    for(size_t i = 0; i < 3; ++i)
    {
        ASSERT_EQ_FMT((int)i, data[i], "%d");
    }
    ASSERT_EQ(dsc_vector_front(cvec), dsc_vector_data(cvec));
    ASSERT_EQ(dsc_vector_back(cvec), dsc_vector_at(cvec, 2));
    ASSERT_EQ(nullptr, dsc_vector_at(cvec, 3));
    ASSERT_EQ(nullptr, dsc_vector_at(cvec, SIZE_MAX));
    PASS();
}

SUITE(vector_access)
{
    SET_SETUP(setup_int_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    SHUFFLE_TESTS(12345, {
        RUN_TEST(accessors_preserve_pointer_types);
        RUN_TEST(accessors_on_empty_return_null);
        RUN_TEST(front_back_data_at);
        RUN_TEST(at_allows_modification);
        RUN_TEST(mutable_accessors_update_const_view);
        RUN_TEST(const_accessors);
    });
}

TEST byte_elements_survive_modifications(void)
{
    dsc_vector_init(&vec, sizeof(unsigned char));
    for(unsigned int i = 0; i < 64; ++i)
    {
        unsigned char value = (unsigned char)i;
        ASSERT(dsc_vector_push_back(&vec, &value));
    }
    unsigned char inserted = 255;
    ASSERT(dsc_vector_insert(&vec, 17, &inserted));
    const unsigned char* inserted_element = dsc_vector_at(&vec, 17);
    ASSERT(inserted_element != nullptr);
    ASSERT_EQ_FMT(255U, (unsigned int)*inserted_element, "%u");
    ASSERT(dsc_vector_erase(&vec, 17));
    ASSERT(dsc_vector_reserve(&vec, 128));
    ASSERT(dsc_vector_shrink_to_fit(&vec));
    ASSERT_SIZE(64, dsc_vector_size(&vec));
    ASSERT_SIZE(64, dsc_vector_capacity(&vec));
    for(size_t i = 0; i < 64; ++i)
    {
        const unsigned char* value = dsc_vector_at(&vec, i);
        ASSERT(value != nullptr);
        ASSERT_EQ_FMT((unsigned int)i, (unsigned int)*value, "%u");
    }
    PASS();
}

TEST odd_sized_structs_survive_modifications(void)
{
    struct Sample
    {
        unsigned char bytes[13];
    };
    struct Sample values[40];
    dsc_vector_init(&vec, sizeof(struct Sample));
    ASSERT_SIZE(13, dsc_vector_element_size(&vec));
    for(size_t i = 0; i < 40; ++i)
    {
        for(size_t j = 0; j < sizeof(values[i].bytes); ++j)
        {
            values[i].bytes[j] = (unsigned char)(i * 13 + j);
        }
        ASSERT(dsc_vector_push_back(&vec, &values[i]));
    }
    ASSERT(dsc_vector_shrink_to_fit(&vec));
    ASSERT(dsc_vector_insert(&vec, 17, &values[0]));
    ASSERT(dsc_vector_at(&vec, 17) != nullptr);
    ASSERT_MEM_EQ(&values[0], dsc_vector_at(&vec, 17), sizeof(struct Sample));
    ASSERT(dsc_vector_erase(&vec, 17));
    ASSERT(dsc_vector_reserve(&vec, 100));
    ASSERT(dsc_vector_shrink_to_fit(&vec));
    ASSERT_SIZE(40, dsc_vector_size(&vec));
    for(size_t i = 0; i < 40; ++i)
    {
        const struct Sample* actual = dsc_vector_at(&vec, i);
        ASSERT(actual != nullptr);
        ASSERT_MEM_EQ(&values[i], actual, sizeof(struct Sample));
    }
    PASS();
}

SUITE(vector_element_types)
{
    SET_SETUP(setup_int_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    SHUFFLE_TESTS(12345, {
        RUN_TEST(byte_elements_survive_modifications);
        RUN_TEST(odd_sized_structs_survive_modifications);
    });
}

/* CTest runs these separately so a hang or memory error cannot hide other results. */
TEST push_past_reserved_capacity_one(void)
{
    ASSERT(dsc_vector_reserve(&vec, 1));
    CHECK_CALL(push_ints(2));
    ASSERT_SIZE(2, dsc_vector_size(&vec));
    ASSERT_INT_AT(0, 0);
    ASSERT_INT_AT(1, 1);
    PASS();
}

TEST push_after_shrinking_to_one(void)
{
    CHECK_CALL(push_ints(1));
    ASSERT(dsc_vector_shrink_to_fit(&vec));
    ASSERT_SIZE(1, dsc_vector_capacity(&vec));
    int value = 42;
    ASSERT(dsc_vector_push_back(&vec, &value));
    ASSERT_SIZE(2, dsc_vector_size(&vec));
    ASSERT_INT_AT(0, 0);
    ASSERT_INT_AT(42, 1);
    PASS();
}

TEST shrink_allocated_empty_vector(void)
{
    ASSERT(dsc_vector_reserve(&vec, 8));
    CHECK_CALL(push_ints(3));
    dsc_vector_clear(&vec);
    ASSERT(dsc_vector_shrink_to_fit(&vec));
    ASSERT_SIZE(0, dsc_vector_size(&vec));
    ASSERT_SIZE(0, dsc_vector_capacity(&vec));
    ASSERT_EQ(nullptr, dsc_vector_data(&vec));
    CHECK_CALL(push_ints(2));
    ASSERT_INT_AT(1, 1);
    PASS();
}

SUITE(vector_regressions)
{
    SET_SETUP(setup_int_vector, nullptr);
    SET_TEARDOWN(teardown_vector, nullptr);
    RUN_TEST(push_past_reserved_capacity_one);
    RUN_TEST(push_after_shrinking_to_one);
    RUN_TEST(shrink_allocated_empty_vector);
}

GREATEST_MAIN_DEFS();

int main(int argc, char** argv)
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(vector_lifecycle);
    RUN_SUITE(vector_capacity);
    RUN_SUITE(vector_modifiers);
    RUN_SUITE(vector_access);
    RUN_SUITE(vector_element_types);
    RUN_SUITE(vector_regressions);
    GREATEST_MAIN_END();
}
