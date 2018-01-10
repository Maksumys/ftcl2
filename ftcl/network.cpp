#include "ftcl/network.hpp"
#include "ftcl/console/log.hpp"
#include <vector>

namespace ftcl
{
    NetworkModule::NetworkModule( )
    {
    }

    MPI_Comm NetworkModule::getParentComm( )
    {
        MPI_Comm bufcomm;
        MPI_Comm_get_parent( &bufcomm );
        return bufcomm;
    }

    void NetworkModule::initialize( int argc, char **argv )
    {
        gargv = argv;
        int initialized;
        MPI_Initialized( &initialized );
        if( initialized == 0 )
        {
            MPI_Init( &argc, &argv );
            MPI_Comm_get_parent( &world );
            if( world == MPI_COMM_NULL )
            {
                std::cout << "Q1" << std::endl;
                MPI_Comm_dup( MPI_COMM_WORLD, &world );
                MPI_Comm_size( world, &size );
                MPI_Comm_rank( world, &rank );
                MPI_Comm_set_errhandler( world, MPI_ERRORS_RETURN );
                char __name[ MPI_MAX_PROCESSOR_NAME ];
                int length = 0;
                MPI_Get_processor_name( __name, &length );
                name = __name;

            }
            else
            {
                std::cout << "Q2" << std::endl;
                MPIX_Comm_replace( MPI_COMM_NULL, &world );
                MPI_Comm_size( world, &size );
                MPI_Comm_rank( world, &rank );
                MPI_Comm_set_errhandler( world, MPI_ERRORS_RETURN );
                char __name[ MPI_MAX_PROCESSOR_NAME ];
                int length = 0;
                MPI_Get_processor_name( __name, &length );
                name = __name;
            }
        }
    }

    void NetworkModule::Spawn( )
    {
        MPI_Comm bufWorld;
        MPIX_Comm_replace( world, &bufWorld );
        MPI_Comm_free( &world );
        world = bufWorld;
    }

    int NetworkModule::MPIX_Comm_replace( MPI_Comm comm, MPI_Comm *newcomm )
    {
        std::cout << "MPIX REPLACE: " << rank << std::endl;
        MPI_Comm icomm; // коммуникатор после очищение но до добавления процессов
        MPI_Comm scomm; // локальный коммуникатор для каждой стороны icomm???восстановленных
        MPI_Comm mcomm; // интракомм слияние
        if( comm == MPI_COMM_NULL )
        {
            MPI_Comm_get_parent( &icomm );
            scomm = MPI_COMM_WORLD;
        }
        else
        {
            MPIX_Comm_shrink( comm, &scomm );
            int ns;
            int nc;
            MPI_Comm_size( scomm, &ns );
            MPI_Comm_size( comm, &nc );
            auto nd = nc - ns;
            if( nd == 0 )
            {
                MPI_Comm_free( &scomm );
                *newcomm = comm;
                return MPI_SUCCESS;
            }
            MPI_Comm_set_errhandler( scomm, MPI_ERRORS_RETURN );
            auto rc = MPI_Comm_spawn(
                    gargv[ 0 ],
                    &gargv[ 1 ],
                    nd,
                    MPI_INFO_NULL,
                    0,
                    scomm,
                    &icomm,
                    MPI_ERRCODES_IGNORE
                );
            int flag = ( MPI_SUCCESS == rc );
            MPIX_Comm_agree( scomm, &flag );
            if( !flag )
            {
                if( rc == MPI_SUCCESS )
                {
                    MPIX_Comm_revoke( icomm );
                    MPI_Comm_free( &icomm );
                }
                MPI_Comm_free( &scomm );
                std::cout << rank << ": comm_spawn failed" << std::endl;
                return -1;
            }
        }

        int rc = MPI_Intercomm_merge( icomm, 1, &mcomm );
        int rflag = ( MPI_SUCCESS == rc );
        int flag = ( MPI_SUCCESS == rc );
        MPIX_Comm_agree( scomm, &flag );
        if( MPI_COMM_WORLD != scomm )
            MPI_Comm_free( &scomm );
        MPIX_Comm_agree( icomm, &rflag );
        MPI_Comm_free( &icomm );
        if( !( flag && rflag ) )
        {
            if( rc == MPI_SUCCESS )
                MPI_Comm_free( &mcomm );
            std::cout << rank << ": Intercomm_merge failed" << std::endl;
            return -1;
        }

        /* restore the error handler */
        if( MPI_COMM_NULL != comm )
        {
            MPI_Errhandler errh;
            MPI_Comm_get_errhandler( comm, &errh );
            MPI_Comm_set_errhandler( mcomm, errh );
        }
        *newcomm = mcomm;
        MPI_Comm_rank( mcomm, &rank );
        return MPI_SUCCESS;
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
        )
    {
        std::cout << "IN send " << std::endl;
        if( __toRank == rank )
            throw exception::Attempt_to_send_to_oneself( __FILE__, __LINE__ );
        MPI_Request request;
        while( true )
        {
            std::unique_lock< std::mutex > guard( mutex );
            auto rc = MPI_Isend(
                    ( void* )__data.data( ),
                    static_cast< int >( __data.size( ) ),
                    MPI_CHAR,
                    __toRank,
                    static_cast< int >( __typeMessage ),
                    world,
                    &request
                );
            if( rc != MPI_SUCCESS )
            {
                std::cout << "IN send replace" << std::endl;
                Spawn( );
            }
            else
                break;
        }
        return request;
    }

    std::vector< char >
    NetworkModule::getMessage(
            MPI_Status &status
        )
    {
        std::vector< char > buf;
        while( true )
        {
            std::unique_lock< std::mutex > guard( mutex );
            int count;
            MPI_Get_count( &status, MPI_CHAR, &count );
            if( count == 0 )
                throw exception::Illegal_size_recv_message( __FILE__, __LINE__ );
            buf.resize( count );

            MPI_Status recvStatus;
            auto rc = MPI_Recv(
                    buf.data( ),
                    count,
                    MPI_CHAR,
                    status.MPI_SOURCE,
                    status.MPI_TAG,
                    world,
                    &recvStatus
                );
            if( rc != MPI_SUCCESS )
            {
                Spawn( );
                std::cout << "IN get message replace" << std::endl;
            }
            else
                break;
        }
        return buf;
    }

    bool NetworkModule::test( NetworkModule::Request &request, NetworkModule::Status &status )
    {
        int testVar;
        while( true )
        {
            std::unique_lock< std::mutex > guard( mutex );
            auto rc = MPI_Test( &request, &testVar, &status );
            if( rc != MPI_SUCCESS )
                Spawn( );
            else
                break;
        }
        return testVar != 0;
    }

    void NetworkModule::cancel(NetworkModule::Request &request)
    {
        while( true )
        {
            std::unique_lock< std::mutex > guard( mutex );
            auto rc = MPI_Cancel( &request );
            if( rc != MPI_SUCCESS )
                Spawn( );
            else
                break;
        }
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

        MPI_Status status;
        int messageFounded;
        while( true )
        {
            std::unique_lock< std::mutex > guard( mutex );
            int rc;
            if( source == -1 )
                rc = MPI_Iprobe(
                        MPI_ANY_SOURCE,
                        static_cast< int >( typeMessage ),
                        world,
                        &messageFounded,
                        &status
                    );
            else
                rc = MPI_Iprobe(
                        source,
                        static_cast< int >( typeMessage ),
                        world,
                        &messageFounded,
                        &status
                    );
            if( rc != MPI_SUCCESS )
            {
                std::cout << "IN check message replace" << std::endl;
                Spawn( );
            }
            else
                break;
        }
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

        MPI_Request request;
        while( true )
        {
            std::unique_lock< std::mutex > guard( mutex );
            auto rc = MPI_Isend(
                    &empty,
                    1,
                    MPI_INT,
                    __toRank,
                    static_cast< int >( __typeMessage ),
                    world,
                    &request
            );
            if( rc != MPI_SUCCESS )
                Spawn( );
            else
                break;
        }
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
            auto rc = MPI_Test( &request, &testVar, &status );
            if( rc != MPI_SUCCESS )
                Spawn( );
            else
                break;

            if( testVar != 0 )
                break;

            std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
            auto end = std::chrono::steady_clock::now( );
            if( std::chrono::duration_cast< std::chrono::seconds >( end - start ).count( ) > sec )
            {
                std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
                auto rc = MPI_Cancel( &request );
                if( rc != MPI_SUCCESS )
                    Spawn( );
                throw exception::Error_worker_logger( __FILE__, __LINE__ );
            }
        }
    }
}



