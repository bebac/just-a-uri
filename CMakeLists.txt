cmake_minimum_required(VERSION 3.21)
project(just-a-uri VERSION 0.1.0 LANGUAGES CXX)

# Define the Library
add_library(just-a-uri STATIC
    source/uri.cpp
    # Add other .cpp files here
)

# Namespaced alias so consumers can link just-a-uri::just-a-uri
add_library(just-a-uri::just-a-uri ALIAS just-a-uri)

# Set up Include Paths
target_include_directories(just-a-uri
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_compile_features(just-a-uri PRIVATE cxx_std_20)

# The one knob controlling whether the test suite is built. Standalone
# builds default to the standard BUILD_TESTING; when consumed via
# add_subdirectory()/FetchContent it defaults to OFF, so a consumer never
# gets our test targets unless it asks. To build the tests inside a
# consumer's build:
#   cmake .. -DJUST_A_URI_BUILD_TESTING=ON
# Safe because the test target and CTest test names are project-prefixed.
if(PROJECT_IS_TOP_LEVEL)
    include(CTest)  # defines the standard BUILD_TESTING option (default ON)
    set(JUST_A_URI_BUILD_TESTING_DEFAULT ${BUILD_TESTING})
else()
    set(JUST_A_URI_BUILD_TESTING_DEFAULT OFF)
endif()

option(JUST_A_URI_BUILD_TESTING "Build the just-a-uri test suite" ${JUST_A_URI_BUILD_TESTING_DEFAULT})

if(JUST_A_URI_BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
