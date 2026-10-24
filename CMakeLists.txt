cmake_minimum_required(VERSION 3.25)
project(pastar VERSION 0.1.0 LANGUAGES CXX)

# Options
option(PASTAR_BUILD_TESTS        "Build the tests"                         ON)
option(PASTAR_WARNINGS_AS_ERRORS "Treat warnings as errors"                OFF)
option(PASTAR_DEBUG              "Enable debug traces (do not benchmark)"  OFF)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Warnings
add_library(pastar_warnings INTERFACE)
target_compile_options(pastar_warnings INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang>:
        -Wall -Wextra -Wpedantic
        -Wconversion -Wsign-conversion
        -Wshadow -Wnon-virtual-dtor -Wold-style-cast
        -Wnull-dereference -Wdouble-promotion
        $<$<BOOL:${PASTAR_WARNINGS_AS_ERRORS}>:-Werror>
    >
)

# Library
add_library(pastar INTERFACE)
add_library(pastar::pastar ALIAS pastar)
target_include_directories(pastar INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)
target_compile_features(pastar INTERFACE cxx_std_20)
if(PASTAR_DEBUG)
  target_compile_definitions(pastar INTERFACE PASTAR_DEBUG)
endif()

# Tests
if(PASTAR_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
