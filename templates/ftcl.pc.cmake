# this gives access to the ftcl header files
prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${CMAKE_INSTALL_PREFIX}
libdir=${CMAKE_INSTALL_PREFIX}/lib
includedir=${CMAKE_INSTALL_PREFIX}/include

Name: ${PROJECT_NAME}
Description: Fault tolerance cluster library.
Version: ${PROJECT_VERSION}
Requires:
Libs: -L${CMAKE_INSTALL_PREFIX}/lib -lftcl
Cflags: -I${CMAKE_INSTALL_PREFIX}/include

# pkg-config does not understand Cxxflags, etc. So we allow users to
# query them using the --variable option

cxxflags=  -I${CMAKE_INSTALL_PREFIX}/include
fflags= -I${CMAKE_INSTALL_PREFIX}/include
fcflags= -I${CMAKE_INSTALL_PREFIX}/include