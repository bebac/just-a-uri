include(FetchContent)

# Reuse doctest if the parent project already provides it
if(NOT TARGET doctest::doctest)
    FetchContent_Declare(
      doctest
      GIT_REPOSITORY https://github.com/doctest/doctest.git
      GIT_TAG        v2.4.12
    )
    FetchContent_MakeAvailable(doctest)
endif()

# Create the test executable. Target names are global across the whole
# build, so prefix it with the project name to avoid clashes with other
# libraries that also define a "unit_tests" target.
add_executable(${PROJECT_NAME}-unittests
    test_uri.cpp
    test_main.cpp
)

target_compile_features(${PROJECT_NAME}-unittests PRIVATE cxx_std_20)

# Link against your library AND doctest
target_link_libraries(${PROJECT_NAME}-unittests
    PRIVATE
        just-a-uri
        doctest::doctest
)

# CTest test names are global too, so prefix them as well
add_test(NAME ${PROJECT_NAME}-unittests COMMAND ${PROJECT_NAME}-unittests)
