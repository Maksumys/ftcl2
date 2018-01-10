cmake_minimum_required(VERSION 3.5.1)
project(ftcl CXX)
set(PROJECT_VERSION "0.0.1")
option(BUILD_DOC "Builds the doc subproject" ON)
option(BUILD_LIB "Build the library subproject" ON)
option(BUILD_TESTS "Build the tests subproject" ON)
option(BUILD_MPI "Build with mpi" OFF)

set(CMAKE_CXX_STANDARD 17)

if(BUILD_DOC)
    add_subdirectory(doc)
endif()

if(BUILD_LIB)
    add_subdirectory(ftcl)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)

endif()
