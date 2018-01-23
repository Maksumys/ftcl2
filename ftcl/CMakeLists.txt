cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)
set(
        HEADERS
        console/log.cpp
        console/log.hpp
        console/logExtensions.hpp
        console/logger.hpp
        console/loggerBase.cpp
        console/loggerBase.hpp
        console/loggerCommon.cpp
        console/loggerCommon.hpp
        console/loggerMpi.hpp
        console/loggerMpi.cpp
        queue.hpp
        exception.hpp
        console/log.hpp
        ftcl.hpp
        multithread/queue.hpp
        sheduler/sheduler.hpp
        sheduler/shedulerMaster.hpp
        sheduler/shedulerWorker.hpp
        sheduler/status.hpp
        sheduler/status.cpp
        timer.hpp
        message_manager.hpp message.hpp)

find_package(MPI)

set(
        FTCL_COMPILE_OPTIONS
        -Wall
        -Wextra
        -g
        -std=c++17
    )

set(
        FTCL_LIBRARIES
    )

set(
        FTCL_DEFINITION
    )

if(MPI_CXX_FOUND)
    list(APPEND FTCL_DEFINITION -DFTCL_MPI_INCLUDED)
    list(APPEND FTCL_DEFINITION -DNETWORK_ON)
    list(APPEND FTCL_LIBRARIES ${MPI_CXX_LIBRARIES})
    list(APPEND HEADERS
            network.cpp
            network.hpp
            )
    set(NETWORK_ON)
endif()

add_library(${PROJECT_NAME} SHARED ${HEADERS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${FTCL_LIBRARIES})
target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${MPI_CXX_INCLUDE_PATH}
)
target_compile_options(${PROJECT_NAME} PUBLIC ${FTCL_COMPILE_OPTIONS})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${FTCL_DEFINITION})


include(FindPkgConfig)

configure_file(
        "${CMAKE_SOURCE_DIR}/templates/ftcl.pc.cmake"
        "ftcl.pc"
    )

install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/ftcl.pc"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
    )

install(
        TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    )

install(
        DIRECTORY ${CMAKE_SOURCE_DIR}/ftcl/
        DESTINATION include/ftcl
        FILES_MATCHING PATTERN "*.h*"
    )
