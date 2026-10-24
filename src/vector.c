#define _DEFAULT_SOURCE // reallocarray

#include "vector.h"

#include <stdlib.h>
#include <string.h>


/** @brief Grows capacity geometrically to meet the requested minimum. */
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

/** @brief Initializes an empty vector without allocating storage. */
void dsc_vector_init(struct DS_COMMON_Vector* vector, size_t element_size)
{
    *vector = (struct DS_COMMON_Vector){.data = NULL, .size = 0, .capacity = 0, .element_size = element_size};
}

/** @brief Destroys the vector's storage without freeing the structure itself. */
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

/** @brief Removes all elements without releasing storage or changing capacity. */
void dsc_vector_clear(struct DS_COMMON_Vector* vector)
{
    vector->size = 0;
}

/** @brief Reserves storage for at least new_capacity elements without shrinking. */
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

/** @brief Reduces capacity to size, releasing the buffer when empty. */
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

/** @brief Appends a copy of one element, growing storage if necessary. */
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

/** @brief Removes the last element without shrinking capacity. */
bool dsc_vector_pop_back(struct DS_COMMON_Vector* vector)
{
    if(vector->size > 0)
    {
        vector->size--;
        return true;
    }
    return false;
}

/** @brief Inserts a copy of one element before index, shifting later elements. */
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

/** @brief Removes an element and shifts subsequent elements toward the front. */
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

/** @brief Returns a const pointer to the first element, or NULL when empty. */
const void* _impl_dsc_vector_front_const(const struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        return nullptr;
    }
    return vector->data;
}
/** @brief Returns a mutable pointer to the first element, or NULL when empty. */
void* _impl_dsc_vector_front(struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        return nullptr;
    }
    return vector->data;
}

/** @brief Returns a const pointer to the last element, or NULL when empty. */
const void* _impl_dsc_vector_back_const(const struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        return nullptr;
    }
    return (char*)vector->data + (vector->size - 1) * vector->element_size;
}
/** @brief Returns a mutable pointer to the last element, or NULL when empty. */
void* _impl_dsc_vector_back(struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        return nullptr;
    }
    return (char*)vector->data + (vector->size - 1) * vector->element_size;
}

/** @brief Returns a const pointer to the element buffer, or NULL when empty. */
const void* _impl_dsc_vector_data_const(const struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        return nullptr;
    }
    return vector->data;
}
/** @brief Returns a mutable pointer to the element buffer, or NULL when empty. */
void* _impl_dsc_vector_data(struct DS_COMMON_Vector* vector)
{
    if(vector->size == 0)
    {
        return nullptr;
    }
    return vector->data;
}

/** @brief Returns a const element pointer, or NULL if index is out of range. */
const void* _impl_dsc_vector_at_const(const struct DS_COMMON_Vector* vector, size_t index)
{
    if(index >= vector->size)
    {
        return nullptr;
    }
    return (char*)vector->data + index * vector->element_size;
}
/** @brief Returns a mutable element pointer, or NULL if index is out of range. */
void* _impl_dsc_vector_at(struct DS_COMMON_Vector* vector, size_t index)
{
    if(index >= vector->size)
    {
        return nullptr;
    }
    return (char*)vector->data + index * vector->element_size;
}

/** @brief Returns the number of live elements. */
size_t dsc_vector_size(const struct DS_COMMON_Vector* vector)
{
    return vector->size;
}

/** @brief Returns the number of elements the allocated buffer can hold. */
size_t dsc_vector_capacity(const struct DS_COMMON_Vector* vector)
{
    return vector->capacity;
}

/** @brief Returns the element size supplied at initialization. */
size_t dsc_vector_element_size(const struct DS_COMMON_Vector* vector)
{
    return vector->element_size;
}

/** @brief Reports whether the vector contains no elements. */
bool dsc_vector_empty(const struct DS_COMMON_Vector* vector)
{
    return vector->size == 0;
}
