#include "dsc_typed_vector_private.h"

#include <stdbool.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

size_t dsc_typed_vector_private_scaled_capacity(size_t capacity, size_t numerator, size_t denominator, size_t limit)
{
    if(capacity <= SIZE_MAX / numerator)
    {
        const size_t scaled = capacity * numerator / denominator;
        return scaled < limit ? scaled : limit;
    }

    const size_t whole = numerator / denominator;
    const size_t fraction = numerator % denominator;
    if(whole != 0 && capacity > limit / whole)
    {
        return limit;
    }

    const size_t base = capacity * whole;
    const size_t room = limit - base;
    size_t quotient = 0;
    size_t remainder = 0;

    /* Compute capacity * fraction / denominator without overflowing. */
    for(size_t bit = SIZE_MAX / 2 + 1; bit != 0; bit >>= 1)
    {
        const bool carry = remainder >= denominator - remainder;
        remainder = carry ? remainder - (denominator - remainder) : remainder + remainder;
        quotient = quotient * 2 + (size_t)carry;
        if((capacity & bit) != 0)
        {
            if(remainder >= denominator - fraction)
            {
                remainder -= denominator - fraction;
                ++quotient;
            }
            else
            {
                remainder += fraction;
            }
        }
        if(quotient >= room)
        {
            return limit;
        }
    }
    return base + quotient;
}

void* dsc_typed_vector_private_reallocate(void* data, size_t live_bytes, size_t allocation_bytes, size_t alignment)
{
    if(alignment <= alignof(max_align_t))
    {
        return realloc(data, allocation_bytes);
    }

    void* new_data = aligned_alloc(alignment, allocation_bytes);
    if(new_data == nullptr)
    {
        return nullptr;
    }
    if(data != nullptr)
    {
        if(live_bytes != 0)
        {
            memcpy(new_data, data, live_bytes);
        }
        free(data);
    }
    return new_data;
}
