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
