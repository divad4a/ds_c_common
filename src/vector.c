#define _DEFAULT_SOURCE // reallocarray

#include "vector.h"

#include <stdlib.h>
#include <string.h>


static bool _impl_dsc_vector_grow_if_necessary(struct DS_COMMON_Vector* vector, size_t min_capacity)
{
    size_t new_capacity = vector->capacity != 0 ? vector->capacity : 1;
    while(new_capacity < min_capacity)
    {
        const auto growth = new_capacity / 2;
        new_capacity = new_capacity + (growth == 0 ? 1 : growth);
    }
    return dsc_vector_reserve(vector, new_capacity);
}

void dsc_vector_init(struct DS_COMMON_Vector* vector, size_t element_size)
{
    *vector = (struct DS_COMMON_Vector){.data = NULL, .size = 0, .capacity = 0, .element_size = element_size};
}

void dsc_vector_free(struct DS_COMMON_Vector* vector)
{
    if(vector->data)
    {
        free(vector->data);
    }
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
}

void dsc_vector_clear(struct DS_COMMON_Vector* vector)
{
    vector->size = 0;
}

bool dsc_vector_reserve(struct DS_COMMON_Vector* vector, size_t new_capacity)
{
    if(vector->data == nullptr)
    {
        if(!(vector->data = calloc(new_capacity, vector->element_size)))
        {
            return false;
        }
        vector->capacity = new_capacity;
    }
    else if(new_capacity > vector->capacity)
    {
        void* new_data_ptr = nullptr;
        if(!(new_data_ptr = reallocarray(vector->data, new_capacity, vector->element_size)))
        {
            return false;
        }
        vector->data = new_data_ptr;
        vector->capacity = new_capacity;
    }
    return true;
}

bool dsc_vector_shrink_to_fit(struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        free(vector->data);
        vector->data = nullptr;
        vector->capacity = 0;
        return true;
    }

    if(vector->capacity > vector->size)
    {
        void* new_data_ptr = nullptr;
        if(!(new_data_ptr = reallocarray(vector->data, vector->size, vector->element_size)))
        {
            return false;
        }
        vector->data = new_data_ptr;
        vector->capacity = vector->size;
    }
    return true;
}

bool dsc_vector_push_back(struct DS_COMMON_Vector* vector, void* element)
{
    if(!_impl_dsc_vector_grow_if_necessary(vector, vector->size + 1))
    {
        return false;
    }
    memcpy((char*)vector->data + vector->size * vector->element_size, element, vector->element_size);
    vector->size++;
    return true;
}

bool dsc_vector_pop_back(struct DS_COMMON_Vector* vector)
{
    if(vector->size > 0)
    {
        vector->size--;
        return true;
    }
    return false;
}

bool dsc_vector_insert(struct DS_COMMON_Vector* vector, size_t index, void* element)
{
    if(!_impl_dsc_vector_grow_if_necessary(vector, vector->size + 1))
    {
        return false;
    }
    if(index > vector->size)
    {
        return false;
    }
    if(index < vector->size)
    {
        memmove((char*)vector->data + (index + 1) * vector->element_size,
                (char*)vector->data + index * vector->element_size, (vector->size - index) * vector->element_size);
    }
    memcpy((char*)vector->data + index * vector->element_size, element, vector->element_size);
    vector->size++;
    return true;
}

bool dsc_vector_erase(struct DS_COMMON_Vector* vector, size_t index)
{
    if(index >= vector->size)
    {
        return false;
    }
    if(index < vector->size - 1)
    {
        memmove((char*)vector->data + index * vector->element_size,
                (char*)vector->data + (index + 1) * vector->element_size,
                (vector->size - index - 1) * vector->element_size);
    }
    vector->size--;
    return true;
}

const void* _impl_dsc_vector_front_const(const struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        return nullptr;
    }
    return vector->data;
}
void* _impl_dsc_vector_front(struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        return nullptr;
    }
    return vector->data;
}

const void* _impl_dsc_vector_back_const(const struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        return nullptr;
    }
    return (char*)vector->data + (vector->size - 1) * vector->element_size;
}
void* _impl_dsc_vector_back(struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        return nullptr;
    }
    return (char*)vector->data + (vector->size - 1) * vector->element_size;
}

const void* _impl_dsc_vector_data_const(const struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        return nullptr;
    }
    return vector->data;
}
void* _impl_dsc_vector_data(struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        return nullptr;
    }
    return vector->data;
}

const void* _impl_dsc_vector_at_const(const struct DS_COMMON_Vector* vector, size_t index)
{
    if(index >= vector->size)
    {
        return nullptr;
    }
    return (char*)vector->data + index * vector->element_size;
}
void* _impl_dsc_vector_at(struct DS_COMMON_Vector* vector, size_t index)
{
    if(index >= vector->size)
    {
        return nullptr;
    }
    return (char*)vector->data + index * vector->element_size;
}

size_t dsc_vector_size(const struct DS_COMMON_Vector* vector)
{
    return vector->size;
}

size_t dsc_vector_capacity(const struct DS_COMMON_Vector* vector)
{
    return vector->capacity;
}

size_t dsc_vector_element_size(const struct DS_COMMON_Vector* vector)
{
    return vector->element_size;
}

bool dsc_vector_empty(const struct DS_COMMON_Vector* vector)
{
    return vector->size == 0;
}
