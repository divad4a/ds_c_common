# ds_c_common

Common C utilities. Requires CMake 3.28+ and a C23 compiler.

## Build and test

Run from the repository root:

```sh
git submodule update --init --recursive

cmake -S . -B build-dir -DCMAKE_BUILD_TYPE=Debug

cmake --build build-dir -j

ctest --test-dir build-dir --output-on-failure
```

To build without tests, add `-DBUILD_TESTING=OFF` when configuring.

## Typed vectors

Declare each specialization in a user-owned header:

```c
/* int_vector.h */
#ifndef INT_VECTOR_H
#define INT_VECTOR_H

#define DSC_VECTOR_TYPE int
#define DSC_VECTOR_SUFFIX int
#define DSC_VECTOR_INITIAL_CAPACITY 8
#define DSC_VECTOR_GROWTH_NUMERATOR 3
#define DSC_VECTOR_GROWTH_DENOMINATOR 2
#include "dsc_typed_vector.h"

#endif
```

Define it in exactly one source file:

```c
/* int_vector.c */
#include "int_vector.h"

#define DSC_VECTOR_SUFFIX int
#include "dsc_typed_vector_impl.h"
```

Compile and link that source with the application. Other files include only
`int_vector.h`:

```c
#include "int_vector.h"

int first_value(void)
{
    dsc_vector_int_t values = {0};
    if(!dsc_vector_int_push_back(&values, 42))
    {
        return -1;
    }
    int first = *dsc_vector_int_front_const(&values);
    dsc_vector_int_free(&values);
    return first;
}
```

The optional settings default to an initial capacity of 1 and a growth ratio
of 3/2; the ratio must be greater than 1. Automatic allocation is lazy;
`reserve` can allocate immediately. Configuration macros are undefined after
inclusion, so a header can declare multiple specializations. Elements are copied
by value, including any pointers they contain; they are not destroyed on
removal. Over-aligned element types are supported. Accessors return borrowed
pointers, which can be invalidated by modifications. Mutable accessors use names
such as `dsc_vector_int_at`; const accessors use the `_const` suffix. See
[dsc_typed_vector.h](include/dsc_typed_vector.h) for type and lifetime
requirements.

## Valgrind

Install Valgrind (`sudo apt install valgrind` on Ubuntu). Use the Debug
build above without sanitizers:

```sh
cmake -S . -B build-valgrind    \
    -DCMAKE_BUILD_TYPE=Debug    \
    -DBUILD_TESTING=ON          \
    -DCMAKE_C_FLAGS="-O0 -g"

cmake --build build-valgrind -j

valgrind                                            \
    --leak-check=full                               \
    --show-leak-kinds=all                           \
    --errors-for-leak-kinds=all                     \
    --track-origins=yes --error-exitcode=1          \
    ./build-valgrind/tests/ds_c_common_test_dsc_vector
```

A clean run reports no leaks and `ERROR SUMMARY: 0 errors`.
