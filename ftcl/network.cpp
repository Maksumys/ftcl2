#include "network.hpp"

NetworkModule::NetworkModule( )
{
    std::cout << "Impl mpi construct" << std::endl;
    MPI_Init( NULL, NULL );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
}

NetworkModule&
NetworkModule::Instance( )
{
    static NetworkModule singleton;
    return singleton;
}

NetworkModule::Number
NetworkModule::getSize( ) const noexcept
{
    return size;
}

NetworkModule::Number
NetworkModule::getRank( ) const noexcept
{
    return rank;
}

NetworkModule::~NetworkModule( )
{
    MPI_Finalize( );
}
    
    
    
