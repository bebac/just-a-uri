cmake_minimum_required(VERSION 3.15)
project(just-a-uri VERSION 0.1.0 LANGUAGES CXX)

# Define the Library
add_library(just-a-uri STATIC
    source/uri.cpp
    # Add other .cpp files here
)

# Set up Include Paths
target_include_directories(just-a-uri
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_compile_features(just-a-uri PRIVATE cxx_std_20)

option(BUILD_TESTING "Build the testing suite" OFF)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

message(STATUS "BUILD_TESTING value is: ${BUILD_TESTING}")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
    message(STATUS "test/CMakeLists.txt FOUND")
else()
    message(STATUS "test/CMakeLists.txt NOT FOUND")
endif()
