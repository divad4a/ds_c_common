#include "typed_vector_fixtures.h"

#include <stdlib.h>
#include <string.h>

static bool fail_next_allocation;
static size_t aligned_allocation_count;

void typed_vector_test_reset_allocator(void)
{
    fail_next_allocation = false;
    aligned_allocation_count = 0;
}

void typed_vector_test_fail_next_allocation(void)
{
    fail_next_allocation = true;
}

size_t typed_vector_test_aligned_allocation_count(void)
{
    return aligned_allocation_count;
}

static void* typed_vector_test_realloc(void* data, size_t size)
{
    if(fail_next_allocation)
    {
        fail_next_allocation = false;
        return nullptr;
    }
    return realloc(data, size);
}

static void* typed_vector_test_aligned_alloc(size_t alignment, size_t size)
{
    ++aligned_allocation_count;
    if(fail_next_allocation)
    {
        fail_next_allocation = false;
        return nullptr;
    }
    return aligned_alloc(alignment, size);
}

/* Inject allocation failures into the generated implementations. */
#define realloc typed_vector_test_realloc
#define aligned_alloc typed_vector_test_aligned_alloc

#define DSC_VECTOR_SUFFIX int
#include "dsc_typed_vector_impl.h"

#define DSC_VECTOR_SUFFIX byte
#include "dsc_typed_vector_impl.h"

#define DSC_VECTOR_SUFFIX double
#include "dsc_typed_vector_impl.h"

#define DSC_VECTOR_SUFFIX int_ptr
#include "dsc_typed_vector_impl.h"

#define DSC_VECTOR_SUFFIX const_int_ptr
#include "dsc_typed_vector_impl.h"

#define DSC_VECTOR_SUFFIX record
#include "dsc_typed_vector_impl.h"

#define DSC_VECTOR_SUFFIX native_aligned
#include "dsc_typed_vector_impl.h"

#define DSC_VECTOR_SUFFIX aligned_64
#include "dsc_typed_vector_impl.h"

#define DSC_VECTOR_SUFFIX aligned_128
#include "dsc_typed_vector_impl.h"

#define DSC_VECTOR_SUFFIX near_one
#include "dsc_typed_vector_impl.h"

#undef aligned_alloc
#undef realloc

#if defined(DSC_VECTOR_SUFFIX) || defined(DSC_TYPED_VECTOR_PRIVATE_NAME) || defined(DSC_TYPED_VECTOR_PRIVATE_BASE) ||    \
    defined(DSC_TYPED_VECTOR_PRIVATE_CAT) || defined(DSC_TYPED_VECTOR_PRIVATE_CAT_IMPL)
#error "Typed vector implementation macros must not leak"
#endif
