/**
 * @file
 * @brief Declares a type-specific vector. Include once per specialization.
 *
 * Define DSC_VECTOR_TYPE as a complete, unqualified, assignable object type
 * and DSC_VECTOR_SUFFIX as a unique identifier before including this file.
 * Array and atomic element types are not supported.
 *
 * Optional integer constant macros:
 * - DSC_VECTOR_INITIAL_CAPACITY: first automatic capacity (default 1).
 * - DSC_VECTOR_GROWTH_NUMERATOR: growth ratio numerator (default 3).
 * - DSC_VECTOR_GROWTH_DENOMINATOR: growth ratio denominator (default 2).
 *
 * The capacity and denominator must be positive; the numerator must be
 * greater than the denominator. Automatic growth rounds the ratio down and
 * increases capacity by at least one. Configuration macros are undefined
 * after inclusion. Names are dsc_vector_SUFFIX_t, dsc_vector_SUFFIX_value_t
 * and dsc_vector_SUFFIX_*. Element size is known from the value type.
 * Include dsc_typed_vector_impl.h once in a .c file to define the functions,
 * and link the implementation with ds_c_common.
 *
 * Elements are copied by value; pointed-to objects are not owned. Initialize
 * vectors with {0} or the generated init function. Do not copy an owning
 * vector. Arguments must be non-NULL and initialized. Operations are not
 * synchronized. Over-aligned types use aligned_alloc(); allocation can fail
 * if the allocator cannot satisfy their alignment.
 *
 * Reallocation invalidates all element pointers. Without reallocation,
 * insertion and erasure invalidate pointers at and after the affected index.
 * Pointers to removed elements are invalid.
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdalign.h>
#include <stdint.h>

#ifndef DSC_VECTOR_TYPE
#error "Define DSC_VECTOR_TYPE before including dsc_typed_vector.h"
#endif

#ifndef DSC_VECTOR_SUFFIX
#error "Define DSC_VECTOR_SUFFIX before including dsc_typed_vector.h"
#endif

#ifndef DSC_VECTOR_INITIAL_CAPACITY
#define DSC_VECTOR_INITIAL_CAPACITY 1
#endif

#ifndef DSC_VECTOR_GROWTH_NUMERATOR
#define DSC_VECTOR_GROWTH_NUMERATOR 3
#endif

#ifndef DSC_VECTOR_GROWTH_DENOMINATOR
#define DSC_VECTOR_GROWTH_DENOMINATOR 2
#endif

#define DSC_TYPED_VECTOR_PRIVATE_CAT_IMPL(a, b) a##b
#define DSC_TYPED_VECTOR_PRIVATE_CAT(a, b) DSC_TYPED_VECTOR_PRIVATE_CAT_IMPL(a, b)
#define DSC_TYPED_VECTOR_PRIVATE_BASE DSC_TYPED_VECTOR_PRIVATE_CAT(dsc_vector_, DSC_VECTOR_SUFFIX)
#define DSC_TYPED_VECTOR_PRIVATE_NAME(name) DSC_TYPED_VECTOR_PRIVATE_CAT(DSC_TYPED_VECTOR_PRIVATE_BASE, name)

/** @brief Element type of this specialization. */
typedef DSC_VECTOR_TYPE DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t);

static_assert(!_Generic((DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)*)0,
                        DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t) const*: 1, default: 0),
              "The element type must not be const-qualified");
static_assert(!_Generic((DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)*)0,
                        DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t) volatile*: 1, default: 0),
              "The element type must not be volatile-qualified");
static_assert((DSC_VECTOR_INITIAL_CAPACITY) > 0 && (DSC_VECTOR_INITIAL_CAPACITY) % 1 == 0,
              "DSC_VECTOR_INITIAL_CAPACITY must be a positive integer");
static_assert((uintmax_t)(DSC_VECTOR_INITIAL_CAPACITY) <= SIZE_MAX / sizeof(DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)),
              "DSC_VECTOR_INITIAL_CAPACITY is too large for the element type");
static_assert((DSC_VECTOR_GROWTH_NUMERATOR) > 0 && (DSC_VECTOR_GROWTH_NUMERATOR) % 1 == 0,
              "DSC_VECTOR_GROWTH_NUMERATOR must be a positive integer");
static_assert((DSC_VECTOR_GROWTH_DENOMINATOR) > 0 && (DSC_VECTOR_GROWTH_DENOMINATOR) % 1 == 0,
              "DSC_VECTOR_GROWTH_DENOMINATOR must be a positive integer");
static_assert((uintmax_t)(DSC_VECTOR_GROWTH_NUMERATOR) <= SIZE_MAX,
              "DSC_VECTOR_GROWTH_NUMERATOR must fit in size_t");
static_assert((uintmax_t)(DSC_VECTOR_GROWTH_DENOMINATOR) <= SIZE_MAX,
              "DSC_VECTOR_GROWTH_DENOMINATOR must fit in size_t");
static_assert((uintmax_t)(DSC_VECTOR_GROWTH_NUMERATOR) > (uintmax_t)(DSC_VECTOR_GROWTH_DENOMINATOR),
              "The vector growth ratio must be greater than one");
static_assert(sizeof(DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)) % alignof(DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)) == 0,
              "The element size must be a multiple of its alignment");

/** @brief Vector storage and bookkeeping. Treat fields as implementation state. */
typedef struct DSC_TYPED_VECTOR_PRIVATE_BASE
{
    DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)* data; /**< Allocated buffer, or NULL. */
    size_t size;                                  /**< Number of live elements. */
    size_t capacity;                              /**< Buffer capacity in elements. */
} DSC_TYPED_VECTOR_PRIVATE_NAME(_t);

static inline size_t DSC_TYPED_VECTOR_PRIVATE_NAME(_private_initial_capacity)(void)
{
    return (size_t)(DSC_VECTOR_INITIAL_CAPACITY);
}

static inline size_t DSC_TYPED_VECTOR_PRIVATE_NAME(_private_growth_numerator)(void)
{
    return (size_t)(DSC_VECTOR_GROWTH_NUMERATOR);
}

static inline size_t DSC_TYPED_VECTOR_PRIVATE_NAME(_private_growth_denominator)(void)
{
    return (size_t)(DSC_VECTOR_GROWTH_DENOMINATOR);
}

/**
 * @brief Initializes an empty vector without allocating.
 * @param[out] vector Vector to initialize; it must not own an allocation.
 */
void DSC_TYPED_VECTOR_PRIVATE_NAME(_init)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector);

/**
 * @brief Releases storage and resets the vector.
 * @param[in,out] vector Vector to free.
 * @note Does not destroy element contents.
 */
void DSC_TYPED_VECTOR_PRIVATE_NAME(_free)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector);

/**
 * @brief Removes all elements without releasing storage.
 * @param[in,out] vector Vector to clear.
 * @note Does not destroy element contents.
 */
void DSC_TYPED_VECTOR_PRIVATE_NAME(_clear)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector);

/**
 * @brief Reserves at least new_capacity elements without shrinking.
 * @param[in,out] vector Vector to grow.
 * @param[in] new_capacity Requested capacity in elements.
 * @retval true The requested capacity is available.
 * @retval false The allocation failed or the capacity is too large; unchanged.
 */
bool DSC_TYPED_VECTOR_PRIVATE_NAME(_reserve)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector, size_t new_capacity);

/**
 * @brief Reduces capacity to size, releasing storage when empty.
 * @param[in,out] vector Vector to shrink.
 * @retval true Capacity equals size.
 * @retval false Allocation failed; the vector is unchanged.
 */
bool DSC_TYPED_VECTOR_PRIVATE_NAME(_shrink_to_fit)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector);

/**
 * @brief Appends a copy of value, growing if needed.
 * @param[in,out] vector Destination vector.
 * @param[in] value Element to append.
 * @retval true The element was appended.
 * @retval false Allocation failed or the capacity limit was reached; unchanged.
 * @note value may be copied from this vector.
 */
bool DSC_TYPED_VECTOR_PRIVATE_NAME(_push_back)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector,
                                               DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t) value);

/**
 * @brief Removes the last element without shrinking.
 * @param[in,out] vector Vector to modify.
 * @retval true An element was removed.
 * @retval false The vector was empty.
 */
bool DSC_TYPED_VECTOR_PRIVATE_NAME(_pop_back)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector);

/**
 * @brief Inserts a copy of value before index, shifting later elements.
 * @param[in,out] vector Destination vector.
 * @param[in] index Insertion position in [0, size]; size appends.
 * @param[in] value Element to insert.
 * @retval true The element was inserted.
 * @retval false Invalid index, allocation failure or capacity limit; unchanged.
 * @note value may be copied from this vector.
 */
bool DSC_TYPED_VECTOR_PRIVATE_NAME(_insert)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector, size_t index,
                                            DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t) value);

/**
 * @brief Removes the element at index and shifts later elements.
 * @param[in,out] vector Vector to modify.
 * @param[in] index Element index in [0, size).
 * @retval true The element was removed.
 * @retval false The index is invalid; the vector is unchanged.
 */
bool DSC_TYPED_VECTOR_PRIVATE_NAME(_erase)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector, size_t index);

/**
 * @brief Returns a mutable pointer to the first element, or NULL when empty.
 * @param[in] vector Vector to access.
 * @return Borrowed element pointer.
 */
DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)* DSC_TYPED_VECTOR_PRIVATE_NAME(_front)(
    DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector);

/**
 * @brief Returns a const pointer to the first element, or NULL when empty.
 * @param[in] vector Vector to access.
 * @return Borrowed element pointer.
 */
DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t) const* DSC_TYPED_VECTOR_PRIVATE_NAME(_front_const)(
    DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector);

/**
 * @brief Returns a mutable pointer to the last element, or NULL when empty.
 * @param[in] vector Vector to access.
 * @return Borrowed element pointer.
 */
DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)* DSC_TYPED_VECTOR_PRIVATE_NAME(_back)(
    DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector);

/**
 * @brief Returns a const pointer to the last element, or NULL when empty.
 * @param[in] vector Vector to access.
 * @return Borrowed element pointer.
 */
DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t) const* DSC_TYPED_VECTOR_PRIVATE_NAME(_back_const)(
    DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector);

/**
 * @brief Returns the mutable buffer, or NULL when empty.
 * @param[in] vector Vector to access.
 * @return Borrowed buffer pointer.
 */
DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)* DSC_TYPED_VECTOR_PRIVATE_NAME(_data)(
    DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector);

/**
 * @brief Returns the const buffer, or NULL when empty.
 * @param[in] vector Vector to access.
 * @return Borrowed buffer pointer.
 */
DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t) const* DSC_TYPED_VECTOR_PRIVATE_NAME(_data_const)(
    DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector);

/**
 * @brief Returns a mutable element pointer, or NULL if index is invalid.
 * @param[in] vector Vector to access.
 * @param[in] index Zero-based element index.
 * @return Borrowed element pointer.
 */
DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t)* DSC_TYPED_VECTOR_PRIVATE_NAME(_at)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t)* vector,
                                                                            size_t index);

/**
 * @brief Returns a const element pointer, or NULL if index is invalid.
 * @param[in] vector Vector to access.
 * @param[in] index Zero-based element index.
 * @return Borrowed element pointer.
 */
DSC_TYPED_VECTOR_PRIVATE_NAME(_value_t) const* DSC_TYPED_VECTOR_PRIVATE_NAME(_at_const)(
    DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector, size_t index);

/**
 * @brief Returns the number of live elements.
 * @param[in] vector Vector to query.
 * @return Element count.
 */
size_t DSC_TYPED_VECTOR_PRIVATE_NAME(_size)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector);

/**
 * @brief Returns the buffer capacity.
 * @param[in] vector Vector to query.
 * @return Capacity in elements.
 */
size_t DSC_TYPED_VECTOR_PRIVATE_NAME(_capacity)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector);

/**
 * @brief Reports whether the vector contains no elements.
 * @param[in] vector Vector to query.
 * @return true if size is zero; false otherwise.
 */
bool DSC_TYPED_VECTOR_PRIVATE_NAME(_empty)(DSC_TYPED_VECTOR_PRIVATE_NAME(_t) const* vector);

#undef DSC_TYPED_VECTOR_PRIVATE_NAME
#undef DSC_TYPED_VECTOR_PRIVATE_BASE
#undef DSC_TYPED_VECTOR_PRIVATE_CAT
#undef DSC_TYPED_VECTOR_PRIVATE_CAT_IMPL
#undef DSC_VECTOR_GROWTH_DENOMINATOR
#undef DSC_VECTOR_GROWTH_NUMERATOR
#undef DSC_VECTOR_INITIAL_CAPACITY
#undef DSC_VECTOR_SUFFIX
#undef DSC_VECTOR_TYPE
