#ifndef DS_COMMON_TYPED_VECTOR_PRIVATE_H
#define DS_COMMON_TYPED_VECTOR_PRIVATE_H

#include <stddef.h>

/* Internal helpers called by generated implementations. */
size_t dsc_typed_vector_private_scaled_capacity(size_t capacity, size_t numerator, size_t denominator, size_t limit);
void* dsc_typed_vector_private_reallocate(void* data, size_t live_bytes, size_t allocation_bytes, size_t alignment);

#endif // DS_COMMON_TYPED_VECTOR_PRIVATE_H
