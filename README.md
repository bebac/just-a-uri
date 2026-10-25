# just-a-uri
A simple C++20 URI

```bash
mkdir build && cd build
cmake .. -DBUILD_TESTING=ON
cmake --build .

# Run tests.
ctest -V

```

## Using just-a-uri in another project

Add the library with `add_subdirectory()` or `FetchContent` and link
`just-a-uri` (or the `just-a-uri::just-a-uri` alias). Its test suite is not
built when the library is consumed. To build the tests inside your own
build:

```bash
cmake .. -DJUST_A_URI_BUILD_TESTING=ON
```

