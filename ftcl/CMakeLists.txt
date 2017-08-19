set(HEADERS
    network.cpp
    console/log.cpp
    network.hpp
    queue.hpp
    exception.hpp
    console/log.hpp
    ftcl.hpp
    multithread/queue.hpp
)

find_package(Mpi REQUIRED)

add_library(${PROJECT_NAME} SHARED ${HEADERS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${MPI_CXX_LIBRARIES})
target_compile_options(${PROJECT_NAME} PUBLIC -std=c++11 )
target_include_directories(
        ${PROJECT_NAME}
        PUBLIC ${MPI_CXX_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}
    )

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
