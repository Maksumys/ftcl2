#include "ftcl/network.hpp"
#include "ftcl/console/log.hpp"
#include <vector>

#ifdef FTCL_MPI_INCLUDED

namespace ftcl
{
    NetworkModule::NetworkModule( )
    {
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

    bool
    NetworkModule::master( ) const noexcept
    {
        return rank == 0;
    }

    MPI_Request
    NetworkModule::send(
            const std::string &__data,
            const std::uint64_t __toRank,
            const TypeMessage __typeMessage
        ) const
    {

        MPI_Request request;
        MPI_Isend(
                __data.data( ),
                static_cast< int >( __data.size( ) ),
                MPI_CHAR,
                static_cast< int >( __toRank ),
                static_cast< int >( __typeMessage ),
                MPI_COMM_WORLD,
                &request
            );
        return request;
    }

    std::vector< char > NetworkModule::getMessage( const MPI_Status &status )
    {
        int count;
        MPI_Get_count( &status, MPI_CHAR, &count );
        if( count == 0 )
            throw exception::Illegal_size_recv_message( __FILE__, __LINE__ );

        std::vector< char > buf;
        buf.resize( count );

        MPI_Status recvStatus;
        MPI_Recv(
                buf.data( ),
                count,
                MPI_CHAR,
                status.MPI_SOURCE,
                status.MPI_TAG,
                MPI_COMM_WORLD,
                &recvStatus
            );

        return buf;
    }

    bool NetworkModule::test( NetworkModule::Request &request, NetworkModule::Status &status )
    {
        int testVar;
        MPI_Test( &request, &testVar, &status );
        return testVar != 0;
    }

    void NetworkModule::cancel(NetworkModule::Request &request)
    {
        MPI_Cancel( &request );
    }

    void NetworkModule::abort()
    {
        MPI_Abort( MPI_COMM_WORLD, 0 );
    }

    std::tuple< bool, MPI_Status >
    NetworkModule::checkMessage( int source, TypeMessage typeMessage )
    {
        MPI_Status status;
        int messageFounded;
        if( source == -1 )
            MPI_Iprobe(
                    MPI_ANY_SOURCE,
                    static_cast< int >( typeMessage ),
                    MPI_COMM_WORLD,
                    &messageFounded,
                    &status
                );
        else
            MPI_Iprobe(
                    source,
                    static_cast< int >( typeMessage ),
                    MPI_COMM_WORLD,
                    &messageFounded,
                    &status
                );
        return std::make_tuple( messageFounded == 1, status );
    }

    NetworkModule::~NetworkModule( )
    {
        MPI_Finalize( );
    }

}

#endif



