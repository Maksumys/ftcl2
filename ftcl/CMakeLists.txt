set(HEADERS
    console/log.cpp
    queue.hpp
    exception.hpp
    console/log.hpp
    ftcl.hpp
    multithread/queue.hpp
    sheduler/sheduler.hpp
)


set(FTCL_COMPILE_OPTIONS
        -std=c++1z
        -Wall
    )

set(FTCL_LIBRARIES
    )

set(FTCL_DEFINITION
   )

if(WITH_MPI)

    find_package(MPI)
    if(MPI_CXX_FOUND)
        list(APPEND FTCL_DEFINITION -DFTCL_MPI_INCLUDED)
        list(APPEND FTCL_LIBRARIES ${MPI_CXX_LIBRARIES})
        list(APPEND HEADERS
                network.cpp
                network.hpp
            )
    endif()

endif()

add_library(${PROJECT_NAME} SHARED ${HEADERS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${FTCL_LIBRARIES})
target_include_directories(
        ${PROJECT_NAME}
        PUBLIC ${MPI_CXX_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}
    )
target_compile_options(${PROJECT_NAME} PUBLIC ${FTCL_COMPILE_OPTIONS})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${FTCL_DEFINITION})


include(FindPkgConfig)

configure_file("${CMAKE_SOURCE_DIR}/templates/ftcl.pc.cmake"
               "ftcl.pc"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ftcl.pc"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
)

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/ftcl/
        DESTINATION include/ftcl
        FILES_MATCHING PATTERN "*.h*"
)
