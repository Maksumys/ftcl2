#ifdef FTCL_MPI_INCLUDED

#include "ftcl/network.hpp"
#include "ftcl/console/log.hpp"
#include <vector>

namespace ftcl
{
    NetworkModule::NetworkModule( )
    {
        int initialized;
        MPI_Initialized( &initialized );
        if( initialized == 0 )
        {
            int i = MPI_Init( NULL, NULL );
            std::cout << " i = " << i << std::endl;
        }
        MPI_Comm_set_errhandler( MPI_COMM_WORLD, MPI_ERRORS_RETURN );


        MPI_Comm_size( MPI_COMM_WORLD, &size );
        MPI_Comm_rank( MPI_COMM_WORLD, &rank );
        char __name[ MPI_MAX_PROCESSOR_NAME ];
        int length = 0;
        MPI_Get_processor_name( __name, &length );
        name = __name;
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
            const Number __toRank,
            const TypeMessage __typeMessage
        ) const
    {
        if( __toRank == rank )
            throw exception::Attempt_to_send_to_oneself( __FILE__, __LINE__ );
        std::unique_lock< std::mutex > guard( mutex );
        MPI_Request request;
        MPI_Isend(
                (void*)__data.data( ),
                static_cast< int >( __data.size( ) ),
                MPI_CHAR,
                __toRank,
                static_cast< int >( __typeMessage ),
                MPI_COMM_WORLD,
                &request
            );
        return request;
    }

    std::vector< char >
    NetworkModule::getMessage(
            MPI_Status &status
        )
    {
        std::unique_lock< std::mutex > guard( mutex );
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
        std::unique_lock< std::mutex > guard( mutex );
        int testVar;
        MPI_Test( &request, &testVar, &status );
        return testVar != 0;
    }

    void NetworkModule::cancel(NetworkModule::Request &request)
    {
        std::unique_lock< std::mutex > guard( mutex );
        MPI_Cancel( &request );
    }

    void NetworkModule::abort( )
    {
        std::unique_lock< std::mutex > guard( mutex );
        MPI_Abort( MPI_COMM_WORLD, 0 );
    }

    std::tuple< bool, MPI_Status >
    NetworkModule::checkMessage( Number source, TypeMessage typeMessage )
    {
        if( source == rank )
            throw exception::Attempt_to_check_to_oneself( __FILE__, __LINE__ );
        std::unique_lock< std::mutex > guard( mutex );
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
        std::unique_lock< std::mutex > guard( mutex );
        MPI_Finalize( );
    }

    MPI_Request NetworkModule::send(const Number __toRank, const TypeMessage __typeMessage)
    {
        if( __toRank >= size  )
            throw exception::Illegal_rank( __FILE__, __LINE__ );

        std::unique_lock< std::mutex > guard( mutex );
        MPI_Request request;
        MPI_Isend(
                &empty,
                1,
                MPI_INT,
                __toRank,
                static_cast< int >( __typeMessage ),
                MPI_COMM_WORLD,
                &request
        );
        return request;
    }

    std::string NetworkModule::getName( )
    {
        return name;
    }

    void NetworkModule::wait( MPI_Request &request, MPI_Status &status, const std::int64_t sec )
    {
        std::unique_lock< std::mutex > guard( mutex );
        auto start = std::chrono::steady_clock::now( );
        while( true )
        {
            int testVar;
            MPI_Test( &request, &testVar, &status );
            if( testVar != 0 )
                break;

            std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
            auto end = std::chrono::steady_clock::now( );
            if( std::chrono::duration_cast< std::chrono::seconds >( end - start ).count( ) > sec )
            {
                std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
                MPI_Cancel( &request );
                throw exception::Error_worker_logger( __FILE__, __LINE__ );
            }
        }
    }

}

#endif



