include(FetchContent)

FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG        v2.4.12
)
FetchContent_MakeAvailable(doctest)

# Create the test executable
add_executable(unit_tests
    test_uri.cpp
    test_main.cpp
)

target_compile_features(unit_tests PRIVATE cxx_std_20)

# Link against your library AND doctest
target_link_libraries(unit_tests
    PRIVATE
        reimagined-dollop
        doctest::doctest
)

# Register the executable with CTest
add_test(NAME AllTests COMMAND unit_tests)
