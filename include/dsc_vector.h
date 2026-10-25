#ifndef DS_COMMON_VECTOR_H
#define DS_COMMON_VECTOR_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @file
 * @brief Contiguous, dynamically sized storage for fixed-size elements.
 *
 * Elements are copied as bytes; pointed-to objects are not owned or destroyed.
 * Unless stated otherwise, vector arguments must be non-NULL and initialized
 * with dsc_vector_init(). Operations are not synchronized.
 *
 * Reallocation invalidates all element pointers. Without reallocation,
 * insertion and erasure invalidate pointers at and after the affected index.
 * Pointers to removed elements are invalid.
 */

/**
 * @brief Vector storage and bookkeeping.
 *
 * Treat fields as implementation state; use the API to modify the vector.
 * Do not shallow-copy an owning vector: copies would share the same allocation.
 */
struct DS_COMMON_Vector
{
    void* data;          /**< Allocated buffer, or NULL if no storage exists. */
    size_t size;         /**< Number of live elements. */
    size_t capacity;     /**< Number of elements the buffer can hold. */
    size_t element_size; /**< Size of each element in bytes. */
};


/**
 * @brief Initializes an empty vector without allocating storage.
 * @param[out] vector Non-NULL vector to initialize.
 * @param[in] element_size Nonzero size of each element in bytes.
 * @pre The vector must not own an allocation; free it before reinitializing.
 */
void dsc_vector_init(struct DS_COMMON_Vector* vector, size_t element_size);

/**
 * @brief Destroys the vector's storage without freeing the structure itself.
 * @param[in,out] vector Vector whose storage is released.
 *
 * Resets data, size and capacity; retains element_size. Does not destroy
 * element contents. All pointers into the buffer become invalid.
 */
void dsc_vector_free(struct DS_COMMON_Vector* vector);

/**
 * @brief Removes all elements without releasing storage or changing capacity.
 * @param[in,out] vector Vector to clear.
 * @note Does not destroy element contents or free pointed-to objects.
 */
void dsc_vector_clear(struct DS_COMMON_Vector* vector);

/**
 * @brief Reserves storage for at least new_capacity elements without shrinking.
 * @param[in,out] vector Vector whose storage may grow.
 * @param[in] new_capacity Requested capacity in elements, not bytes.
 * @retval true Storage is available for the requested capacity.
 * @retval false Allocation failed; the vector is unchanged.
 * @note Size and existing elements are preserved. Reallocation invalidates
 * all pointers into the buffer.
 */
bool dsc_vector_reserve(struct DS_COMMON_Vector* vector, size_t new_capacity);

/**
 * @brief Reduces capacity to size, releasing the buffer when empty.
 * @param[in,out] vector Vector whose storage may shrink.
 * @retval true Capacity equals size.
 * @retval false Allocation failed; the vector is unchanged.
 * @note Preserves elements and element_size. Reallocation or release
 * invalidates all pointers into the buffer.
 */
bool dsc_vector_shrink_to_fit(struct DS_COMMON_Vector* vector);

/**
 * @brief Appends a copy of one element, growing storage if necessary.
 * @param[in,out] vector Destination vector.
 * @param[in] element Non-NULL pointer to at least element_size readable bytes.
 * @pre The source element must not overlap the vector's allocated storage.
 * @retval true The element was appended.
 * @retval false Allocation failed; the vector is unchanged.
 * @note Reallocation invalidates all pointers into the buffer.
 */
bool dsc_vector_push_back(struct DS_COMMON_Vector* vector, void* element);

/**
 * @brief Removes the last element without shrinking capacity.
 * @param[in,out] vector Vector to modify.
 * @retval true The last element was removed.
 * @retval false The vector was empty and remains unchanged.
 * @note Does not destroy element contents.
 */
bool dsc_vector_pop_back(struct DS_COMMON_Vector* vector);

/**
 * @brief Inserts a copy of one element before index, shifting later elements.
 * @param[in,out] vector Destination vector.
 * @param[in] index Insertion position in [0, size]; size appends.
 * @param[in] element Non-NULL pointer to at least element_size readable bytes.
 * @pre The source element must not overlap the vector's allocated storage.
 * @retval true The element was inserted.
 * @retval false The index exceeds size or allocation failed.
 * @note The vector is unchanged on failure.
 * @note On success, elements at and after index shift; reallocation invalidates
 * all pointers into the buffer.
 */
bool dsc_vector_insert(struct DS_COMMON_Vector* vector, size_t index, void* element);

/**
 * @brief Removes an element and shifts subsequent elements toward the front.
 * @param[in,out] vector Vector to modify.
 * @param[in] index Element index in [0, size).
 * @retval true The element was removed.
 * @retval false The index is out of range; the vector is unchanged.
 * @note Capacity is retained. Does not destroy element contents.
 */
bool dsc_vector_erase(struct DS_COMMON_Vector* vector, size_t index);

/**
 * @brief Returns a const pointer to the first element, or NULL when empty.
 * @param[in] vector Vector to access.
 * @return Borrowed const element pointer; see file-level lifetime rules.
 */
const void* dsc_vector_front_const(const struct DS_COMMON_Vector* vector);

/**
 * @brief Returns a mutable pointer to the first element, or NULL when empty.
 * @param[in] vector Vector to access.
 * @return Borrowed mutable element pointer; see file-level lifetime rules.
 */
void* dsc_vector_front_nconst(struct DS_COMMON_Vector* vector);

/**
 * @brief Returns a pointer to the first element, or NULL when empty.
 * @param[in] vector Initialized mutable or const vector pointer.
 * @return void* for a mutable vector; const void* for a const vector.
 * @note The pointer is borrowed; do not free it. See file-level lifetime rules.
 */
#define dsc_vector_front(vector)                                                                                       \
    _Generic((vector),                                                                                                 \
        const struct DS_COMMON_Vector*: dsc_vector_front_const,                                                        \
        struct DS_COMMON_Vector*: dsc_vector_front_nconst)(vector)

/**
 * @brief Returns a const pointer to the last element, or NULL when empty.
 * @param[in] vector Vector to access.
 * @return Borrowed const element pointer; see file-level lifetime rules.
 */
const void* dsc_vector_back_const(const struct DS_COMMON_Vector* vector);

/**
 * @brief Returns a mutable pointer to the last element, or NULL when empty.
 * @param[in] vector Vector to access.
 * @return Borrowed mutable element pointer; see file-level lifetime rules.
 */
void* dsc_vector_back_nconst(struct DS_COMMON_Vector* vector);

/**
 * @brief Returns a pointer to the last element, or NULL when empty.
 * @param[in] vector Initialized mutable or const vector pointer.
 * @return void* for a mutable vector; const void* for a const vector.
 * @note The pointer is borrowed; do not free it. See file-level lifetime rules.
 */
#define dsc_vector_back(vector)                                                                                        \
    _Generic((vector),                                                                                                 \
        const struct DS_COMMON_Vector*: dsc_vector_back_const,                                                         \
        struct DS_COMMON_Vector*: dsc_vector_back_nconst)(vector)

/**
 * @brief Returns a const buffer pointer, or NULL when empty even if allocated.
 * @param[in] vector Vector to access.
 * @return Borrowed const buffer pointer; see file-level lifetime rules.
 */
const void* dsc_vector_data_const(const struct DS_COMMON_Vector* vector);

/**
 * @brief Returns a mutable buffer pointer, or NULL when empty even if allocated.
 * @param[in] vector Vector to access.
 * @return Borrowed mutable buffer pointer; see file-level lifetime rules.
 */
void* dsc_vector_data_nconst(struct DS_COMMON_Vector* vector);

/**
 * @brief Returns the contiguous element buffer, or NULL when empty.
 * @param[in] vector Initialized mutable or const vector pointer.
 * @return void* for a mutable vector; const void* for a const vector.
 * @note Returns NULL for empty vectors even if capacity is nonzero.
 * The pointer is borrowed; do not free it. See file-level lifetime rules.
 */
#define dsc_vector_data(vector)                                                                                        \
    _Generic((vector),                                                                                                 \
        const struct DS_COMMON_Vector*: dsc_vector_data_const,                                                         \
        struct DS_COMMON_Vector*: dsc_vector_data_nconst)(vector)

/**
 * @brief Returns a const element pointer, or NULL if index is out of range.
 * @param[in] vector Vector to access.
 * @param[in] index Zero-based element index.
 * @return Borrowed const element pointer; see file-level lifetime rules.
 */
const void* dsc_vector_at_const(const struct DS_COMMON_Vector* vector, size_t index);

/**
 * @brief Returns a mutable element pointer, or NULL if index is out of range.
 * @param[in] vector Vector to access.
 * @param[in] index Zero-based element index.
 * @return Borrowed mutable element pointer; see file-level lifetime rules.
 */
void* dsc_vector_at_nconst(struct DS_COMMON_Vector* vector, size_t index);

/**
 * @brief Returns a pointer to an element, or NULL if index is out of range.
 * @param[in] vector Initialized mutable or const vector pointer.
 * @param[in] index Zero-based element index.
 * @return void* for a mutable vector; const void* for a const vector.
 * @note The pointer is borrowed; do not free it. See file-level lifetime rules.
 */
#define dsc_vector_at(vector, index)                                                                                   \
    _Generic((vector),                                                                                                 \
        const struct DS_COMMON_Vector*: dsc_vector_at_const,                                                           \
        struct DS_COMMON_Vector*: dsc_vector_at_nconst)(vector, index)

/**
 * @brief Returns the number of live elements.
 * @param[in] vector Vector to query.
 * @return Element count.
 */
size_t dsc_vector_size(const struct DS_COMMON_Vector* vector);

/**
 * @brief Returns the number of elements the allocated buffer can hold.
 * @param[in] vector Vector to query.
 * @return Capacity in elements, not bytes.
 */
size_t dsc_vector_capacity(const struct DS_COMMON_Vector* vector);

/**
 * @brief Returns the element size supplied at initialization.
 * @param[in] vector Vector to query.
 * @return Size of one element in bytes.
 */
size_t dsc_vector_element_size(const struct DS_COMMON_Vector* vector);

/**
 * @brief Reports whether the vector contains no elements.
 * @param[in] vector Vector to query.
 * @return true if size is zero; false otherwise.
 */
bool dsc_vector_empty(const struct DS_COMMON_Vector* vector);


#endif // DS_COMMON_VECTOR_H
