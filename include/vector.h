#ifndef DS_COMMON_VECTOR_H
#define DS_COMMON_VECTOR_H

#include <stdbool.h>
#include <stddef.h>

struct DS_COMMON_Vector
{
    void* data;
    size_t size;
    size_t capacity;
    size_t element_size;
};


void dsc_vector_init(struct DS_COMMON_Vector* vector, size_t element_size);

void dsc_vector_free(struct DS_COMMON_Vector* vector);

void dsc_vector_clear(struct DS_COMMON_Vector* vector);

bool dsc_vector_reserve(struct DS_COMMON_Vector* vector, size_t new_capacity);

bool dsc_vector_shrink_to_fit(struct DS_COMMON_Vector* vector);

bool dsc_vector_push_back(struct DS_COMMON_Vector* vector, void* element);

bool dsc_vector_pop_back(struct DS_COMMON_Vector* vector);

bool dsc_vector_insert(struct DS_COMMON_Vector* vector, size_t index, void* element);

bool dsc_vector_erase(struct DS_COMMON_Vector* vector, size_t index);

const void* _impl_dsc_vector_front_const(const struct DS_COMMON_Vector* vector);
void* _impl_dsc_vector_front(struct DS_COMMON_Vector* vector);
#define dsc_vector_front(vector)                                                                                       \
    _Generic((vector),                                                                                                 \
        const struct DS_COMMON_Vector*: _impl_dsc_vector_front_const,                                                  \
        struct DS_COMMON_Vector*: _impl_dsc_vector_front)(vector)

const void* _impl_dsc_vector_back_const(const struct DS_COMMON_Vector* vector);
void* _impl_dsc_vector_back(struct DS_COMMON_Vector* vector);
#define dsc_vector_back(vector)                                                                                        \
    _Generic((vector),                                                                                                 \
        const struct DS_COMMON_Vector*: _impl_dsc_vector_back_const,                                                   \
        struct DS_COMMON_Vector*: _impl_dsc_vector_back)(vector)

const void* _impl_dsc_vector_data_const(const struct DS_COMMON_Vector* vector);
void* _impl_dsc_vector_data(struct DS_COMMON_Vector* vector);
#define dsc_vector_data(vector)                                                                                        \
    _Generic((vector),                                                                                                 \
        const struct DS_COMMON_Vector*: _impl_dsc_vector_data_const,                                                   \
        struct DS_COMMON_Vector*: _impl_dsc_vector_data)(vector)

const void* _impl_dsc_vector_at_const(const struct DS_COMMON_Vector* vector, size_t index);
void* _impl_dsc_vector_at(struct DS_COMMON_Vector* vector, size_t index);
#define dsc_vector_at(vector, index)                                                                                   \
    _Generic((vector),                                                                                                 \
        const struct DS_COMMON_Vector*: _impl_dsc_vector_at_const,                                                     \
        struct DS_COMMON_Vector*: _impl_dsc_vector_at)(vector, index)

size_t dsc_vector_size(const struct DS_COMMON_Vector* vector);

size_t dsc_vector_capacity(const struct DS_COMMON_Vector* vector);

size_t dsc_vector_element_size(const struct DS_COMMON_Vector* vector);

bool dsc_vector_empty(const struct DS_COMMON_Vector* vector);


#endif // DS_COMMON_VECTOR_H
