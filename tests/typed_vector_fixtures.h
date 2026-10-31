#ifndef DS_COMMON_TYPED_VECTOR_FIXTURES_H
#define DS_COMMON_TYPED_VECTOR_FIXTURES_H

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

struct test_record
{
    int id;
    char name[24];
    struct
    {
        double x;
        double y;
    } position;
    double samples[3];
    int* owner;
};

struct test_native_aligned
{
    alignas(max_align_t) unsigned char bytes[7];
    unsigned int id;
};

struct test_aligned_64
{
    alignas(64) uint64_t id;
    double samples[3];
};

struct test_aligned_128
{
    alignas(128) uint64_t id;
    unsigned char bytes[17];
};

#define DSC_VECTOR_TYPE int
#define DSC_VECTOR_SUFFIX int
#define DSC_VECTOR_INITIAL_CAPACITY 4
#define DSC_VECTOR_GROWTH_NUMERATOR 5
#define DSC_VECTOR_GROWTH_DENOMINATOR 3
#include "dsc_typed_vector.h"

#define DSC_VECTOR_TYPE unsigned char
#define DSC_VECTOR_SUFFIX byte
#include "dsc_typed_vector.h"

#define DSC_VECTOR_TYPE double
#define DSC_VECTOR_SUFFIX double
#include "dsc_typed_vector.h"

#define DSC_VECTOR_TYPE int*
#define DSC_VECTOR_SUFFIX int_ptr
#include "dsc_typed_vector.h"

#define DSC_VECTOR_TYPE const int*
#define DSC_VECTOR_SUFFIX const_int_ptr
#include "dsc_typed_vector.h"

#define DSC_VECTOR_TYPE struct test_record
#define DSC_VECTOR_SUFFIX record
#include "dsc_typed_vector.h"

#define DSC_VECTOR_TYPE struct test_native_aligned
#define DSC_VECTOR_SUFFIX native_aligned
#include "dsc_typed_vector.h"

#define DSC_VECTOR_TYPE struct test_aligned_64
#define DSC_VECTOR_SUFFIX aligned_64
#include "dsc_typed_vector.h"

#define DSC_VECTOR_TYPE struct test_aligned_128
#define DSC_VECTOR_SUFFIX aligned_128
#include "dsc_typed_vector.h"

#define DSC_VECTOR_TYPE unsigned int
#define DSC_VECTOR_SUFFIX near_one
#define DSC_VECTOR_GROWTH_NUMERATOR SIZE_MAX
#define DSC_VECTOR_GROWTH_DENOMINATOR (SIZE_MAX - 1)
#include "dsc_typed_vector.h"

#if defined(DSC_VECTOR_TYPE) || defined(DSC_VECTOR_SUFFIX) || defined(DSC_VECTOR_INITIAL_CAPACITY) ||                   \
    defined(DSC_VECTOR_GROWTH_NUMERATOR) || defined(DSC_VECTOR_GROWTH_DENOMINATOR) ||                                  \
    defined(DSC_TYPED_VECTOR_PRIVATE_NAME) || defined(DSC_TYPED_VECTOR_PRIVATE_BASE) ||                                 \
    defined(DSC_TYPED_VECTOR_PRIVATE_CAT) || defined(DSC_TYPED_VECTOR_PRIVATE_CAT_IMPL)
#error "Typed vector declaration macros must not leak"
#endif

void typed_vector_test_reset_allocator(void);
void typed_vector_test_fail_next_allocation(void);
size_t typed_vector_test_aligned_allocation_count(void);

#endif // DS_COMMON_TYPED_VECTOR_FIXTURES_H
