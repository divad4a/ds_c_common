#include "typed_vector_fixtures.h"

#include "dsc_typed_vector_private.h"

#include <stdlib.h>

void* typed_vector_test_realloc(void* data, size_t size);
void* typed_vector_test_aligned_alloc(size_t alignment, size_t size);
void* typed_vector_test_private_reallocate(void* data, size_t live_bytes, size_t allocation_bytes, size_t alignment);

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

void* typed_vector_test_realloc(void* data, size_t size)
{
    if(fail_next_allocation)
    {
        fail_next_allocation = false;
        return nullptr;
    }
    return realloc(data, size);
}

void* typed_vector_test_aligned_alloc(size_t alignment, size_t size)
{
    if(fail_next_allocation)
    {
        fail_next_allocation = false;
        return nullptr;
    }
    return aligned_alloc(alignment, size);
}

/* Use the test-allocator build only when a failure was requested. */
static void* typed_vector_test_dispatch_reallocate(void* data, size_t live_bytes, size_t allocation_bytes,
                                                  size_t alignment)
{
    if(alignment > alignof(max_align_t))
    {
        ++aligned_allocation_count;
    }
    if(fail_next_allocation)
    {
        return typed_vector_test_private_reallocate(data, live_bytes, allocation_bytes, alignment);
    }
    return dsc_typed_vector_private_reallocate(data, live_bytes, allocation_bytes, alignment);
}

#define dsc_typed_vector_private_reallocate typed_vector_test_dispatch_reallocate

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

#undef dsc_typed_vector_private_reallocate

#if defined(DSC_VECTOR_SUFFIX) || defined(DSC_TYPED_VECTOR_PRIVATE_NAME) || defined(DSC_TYPED_VECTOR_PRIVATE_BASE) ||    \
    defined(DSC_TYPED_VECTOR_PRIVATE_CAT) || defined(DSC_TYPED_VECTOR_PRIVATE_CAT_IMPL)
#error "Typed vector implementation macros must not leak"
#endif
