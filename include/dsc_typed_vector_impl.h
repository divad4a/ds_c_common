/**
 * @file
 * @brief Defines one typed vector specialization.
 *
 * Include the user-owned declaration header first, define DSC_VECTOR_SUFFIX,
 * and include this file in exactly one .c file. Type and growth settings come
 * from the declaration. DSC_VECTOR_SUFFIX is undefined after inclusion.
 * Link the resulting implementation with ds_c_common.
 */

#ifndef DSC_VECTOR_SUFFIX
#error "Define DSC_VECTOR_SUFFIX before including dsc_typed_vector_impl.h"
#endif

#if defined(DSC_VECTOR_TYPE) || defined(DSC_VECTOR_INITIAL_CAPACITY) || defined(DSC_VECTOR_GROWTH_NUMERATOR)           \
    || defined(DSC_VECTOR_GROWTH_DENOMINATOR)
#error "Configure the vector type and growth in its declaration, not its implementation"
#endif

#include "dsc_typed_vector_private.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DSC_TYPED_VECTOR_PRIVATE_CAT_IMPL(a, b) a##b
#define DSC_TYPED_VECTOR_PRIVATE_CAT(a, b) DSC_TYPED_VECTOR_PRIVATE_CAT_IMPL(a, b)
#define DSC_TYPED_VECTOR_PRIVATE_BASE DSC_TYPED_VECTOR_PRIVATE_CAT(dsc_vector_, DSC_VECTOR_SUFFIX)
#define DSC_TYPED_VECTOR_PRIVATE_NAME(name) DSC_TYPED_VECTOR_PRIVATE_CAT(DSC_TYPED_VECTOR_PRIVATE_BASE, name)

static bool DSC_TYPED_VECTOR_PRIVATE_NAME(_private_grow)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector)
{
    const size_t max_capacity = SIZE_MAX / sizeof(DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t));
    if(vector->size >= max_capacity)
    {
        return false;
    }
    if(vector->size < vector->capacity)
    {
        return true;
    }

    size_t new_capacity;
    if(vector->capacity == 0)
    {
        new_capacity = DSC_TYPED_VECTOR_PRIVATE_NAME(_private_initial_capacity)();
    }
    else
    {
        new_capacity = dsc_typed_vector_private_scaled_capacity(
            vector->capacity, DSC_TYPED_VECTOR_PRIVATE_NAME(_private_growth_numerator)(),
            DSC_TYPED_VECTOR_PRIVATE_NAME(_private_growth_denominator)(), max_capacity);
    }
    if(new_capacity <= vector->size)
    {
        new_capacity = vector->size + 1;
    }
    return DSC_TYPED_VECTOR_PRIVATE_NAME(_reserve)(vector, new_capacity);
}

void DSC_TYPED_VECTOR_PRIVATE_NAME(_init)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector)
{
    *vector = (DSC_TYPED_VECTOR_PRIVATE_NAME(_t)){0};
}

void DSC_TYPED_VECTOR_PRIVATE_NAME(_free)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector)
{
    free(vector->data);
    *vector = (DSC_TYPED_VECTOR_PRIVATE_NAME(_t)){0};
}

void DSC_TYPED_VECTOR_PRIVATE_NAME(_clear)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector)
{
    vector->size = 0;
}

bool DSC_TYPED_VECTOR_PRIVATE_NAME(_reserve)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector, size_t new_capacity)
{
    if(new_capacity <= vector->capacity)
    {
        return true;
    }
    if(new_capacity > SIZE_MAX / sizeof(DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)))
    {
        return false;
    }

    DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)* new_data = dsc_typed_vector_private_reallocate(
        vector->data, vector->size * sizeof(DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)),
        new_capacity * sizeof(DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)),
        alignof(DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)));
    if(new_data == nullptr)
    {
        return false;
    }
    vector->data = new_data;
    vector->capacity = new_capacity;
    return true;
}

bool DSC_TYPED_VECTOR_PRIVATE_NAME(_shrink_to_fit)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector)
{
    if(vector->size == 0)
    {
        free(vector->data);
        vector->data = nullptr;
        vector->capacity = 0;
        return true;
    }
    if(vector->size == vector->capacity)
    {
        return true;
    }

    const size_t bytes = vector->size * sizeof(DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t));
    DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)* new_data = dsc_typed_vector_private_reallocate(
        vector->data, bytes, bytes, alignof(DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)));
    if(new_data == nullptr)
    {
        return false;
    }
    vector->data = new_data;
    vector->capacity = vector->size;
    return true;
}

bool DSC_TYPED_VECTOR_PRIVATE_NAME(_push_back)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector,
                                               DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t) value)
{
    if(!DSC_TYPED_VECTOR_PRIVATE_NAME(_private_grow)(vector))
    {
        return false;
    }
    vector->data[vector->size++] = value;
    return true;
}

bool DSC_TYPED_VECTOR_PRIVATE_NAME(_pop_back)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector)
{
    if(vector->size == 0)
    {
        return false;
    }
    --vector->size;
    return true;
}

bool DSC_TYPED_VECTOR_PRIVATE_NAME(_insert)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector, size_t index,
                                            DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t) value)
{
    if(index > vector->size || !DSC_TYPED_VECTOR_PRIVATE_NAME(_private_grow)(vector))
    {
        return false;
    }
    if(index < vector->size)
    {
        memmove(vector->data + index + 1, vector->data + index,
                (vector->size - index) * sizeof(DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)));
    }
    vector->data[index] = value;
    ++vector->size;
    return true;
}

bool DSC_TYPED_VECTOR_PRIVATE_NAME(_erase)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector, size_t index)
{
    if(index >= vector->size)
    {
        return false;
    }
    if(index < vector->size - 1)
    {
        memmove(vector->data + index, vector->data + index + 1,
                (vector->size - index - 1) * sizeof(DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)));
    }
    --vector->size;
    return true;
}

DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)
    * DSC_TYPED_VECTOR_PRIVATE_NAME(_front)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector)
{
    return vector->size == 0 ? nullptr : vector->data;
}

DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)
const* DSC_TYPED_VECTOR_PRIVATE_NAME(_front_const)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector)
{
    return vector->size == 0 ? nullptr : vector->data;
}

DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)
    * DSC_TYPED_VECTOR_PRIVATE_NAME(_back)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector)
{
    return vector->size == 0 ? nullptr : vector->data + vector->size - 1;
}

DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)
const* DSC_TYPED_VECTOR_PRIVATE_NAME(_back_const)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector)
{
    return vector->size == 0 ? nullptr : vector->data + vector->size - 1;
}

DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)
    * DSC_TYPED_VECTOR_PRIVATE_NAME(_data)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector)
{
    return vector->size == 0 ? nullptr : vector->data;
}

DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)
const* DSC_TYPED_VECTOR_PRIVATE_NAME(_data_const)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector)
{
    return vector->size == 0 ? nullptr : vector->data;
}

DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)
    * DSC_TYPED_VECTOR_PRIVATE_NAME(_at)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) * vector, size_t index)
{
    return index >= vector->size ? nullptr : vector->data + index;
}

DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)
const* DSC_TYPED_VECTOR_PRIVATE_NAME(_at_const)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector, size_t index)
{
    return index >= vector->size ? nullptr : vector->data + index;
}

size_t DSC_TYPED_VECTOR_PRIVATE_NAME(_size)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector)
{
    return vector->size;
}

size_t DSC_TYPED_VECTOR_PRIVATE_NAME(_capacity)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector)
{
    return vector->capacity;
}

bool DSC_TYPED_VECTOR_PRIVATE_NAME(_empty)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector)
{
    return vector->size == 0;
}

#undef DSC_TYPED_VECTOR_PRIVATE_NAME
#undef DSC_TYPED_VECTOR_PRIVATE_BASE
#undef DSC_TYPED_VECTOR_PRIVATE_CAT
#undef DSC_TYPED_VECTOR_PRIVATE_CAT_IMPL
#undef DSC_VECTOR_SUFFIX
