if(DEFINED WITH_GOOGLE_TEST)
    if((EXISTS "${WITH_GOOGLE_TEST}/lib/libgtest.a") AND
        (EXISTS "${WITH_GOOGLE_TEST}/lib/libgmock.a"))
        set(GTEST_BOTH_LIBRARIES
            "${WITH_GOOGLE_TEST}/lib/libgtest.a"
            "${WITH_GOOGLE_TEST}/lib/libgtest_main.a"
            )
        set(GTEST_INCLUDE_DIR
            "${WITH_GOOGLE_TEST}/include"
            )
    else()
        message(STATUS "Google test not founded!")
        return()
    endif()

else()
    find_package(GTest)
    if(NOT ${GTEST_FOUND})
        message(STATUS "Google test not founded!")
        return()
    endif()
endif()



include_directories(${CMAKE_SOURCE_DIR} ${GTEST_INCLUDE_DIR})
add_compile_options(-std=c++11)
link_libraries(
        ${GTEST_BOTH_LIBRARIES}
        ${PROJECT_NAME}
        -ldl
    )

add_executable(testQueue testQueue.cpp)
add_test(testQueue testQueue)

add_executable(testMultiQueue testMultiQueue.cpp)
add_test(testMultiQueue testMultiQueue)

add_executable(testLog testLog.cpp)
add_test(testLog testLog)

add_executable(testNetworkModule testNetworkModule.cpp)
add_test(NAME testNetworkModule COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} "2" "./testNetworkModule")
