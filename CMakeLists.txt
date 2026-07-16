cmake_minimum_required(VERSION 3.22)

project(AudioGaussianSplatter
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR
        "In source builds are not allowed. "
        "Please create a 'build' directory and run cmake from there: "
        " mkdir build && cd build && cmake ..")
endif ()

include(cmake/FetchJUCE.cmake)
include(cmake/FetchDependencies.cmake)

include(CTest)

option(BUILD_TESTS "Build the AGS test suite" ON)

add_subdirectory(spiral-dsp)
add_subdirectory(ags-manifold)
add_subdirectory(ags-app)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()

option(BUILD_TOOLS "Build developer preview tools" ON)

if (BUILD_TOOLS)
    add_subdirectory(tools/ManifoldPreview)
endif ()