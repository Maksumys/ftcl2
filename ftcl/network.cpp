#include "ftcl/network.hpp"
#include "ftcl/console/log.hpp"
#include "ftcl/exception.hpp"

namespace ftcl
{
    NetworkModule::NetworkModule( ) = default;


    void print_comm_ranks(MPI_Comm test_comm)
    {
        MPI_Group grp, world_grp;

        MPI_Comm_group(MPI_COMM_WORLD, &world_grp);
        MPI_Comm_group(test_comm, &grp);

        int grp_size;

        MPI_Group_size(grp, &grp_size);

        std::vector< int > ranks( grp_size );
        std::vector< int > world_ranks( grp_size );

        for (int i = 0; i < grp_size; i++)
            ranks[i] = i;

        MPI_Group_translate_ranks(grp, grp_size, ranks.data( ), world_grp, world_ranks.data( ) );

        for (int i = 0; i < grp_size; i++)
            printf("comm[%d] has world rank %d\n", i, world_ranks[i]);

        MPI_Group_free(&grp);
        MPI_Group_free(&world_grp);
    }

    void NetworkModule::initialize( int argc, char **argv )
    {
        std::unique_lock lock( mutex );
        gargv = argv;
        //int initialized;
        //MPI_Initialized( &initialized );
        //if( initialized == 0 )
        {
            MPI_Init( &argc, &argv );
            MPI_Comm_get_parent( &world );
            if( world == MPI_COMM_NULL )
            {
                MPI_Comm_dup( MPI_COMM_WORLD, &world );
                MPI_Comm_size( world, &size );
                MPI_Comm_rank( world, &rank );
                MPI_Comm_set_errhandler( world, MPI_ERRORS_RETURN );
                char __name[ MPI_MAX_PROCESSOR_NAME ];
                int length = 0;
                MPI_Get_processor_name( __name, &length );
                name = __name;


                /// заполнение таблицы виртуальных номеров процессов
                for( int i = 0; i < size; i++ )
                    virtual_ranks.emplace( i, i );
            }
            else
            {
                //MPIX_Comm_replace( MPI_COMM_NULL, &world );
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

    void NetworkModule::spawn( )
    {
        MPI_Comm bufWorld;
        MPIX_Comm_replace( world, &bufWorld );
        MPI_Comm_free( &world );
        world = bufWorld;
        isReplace = true;
    }

    std::vector< std::size_t > NetworkModule::getFailingProc( )
    {
        return failingProc;
    }

    int NetworkModule::MPIX_Comm_replace( MPI_Comm comm, MPI_Comm *newcomm )
    {
        std::cout << "qwe " << std::endl;
        int oldrank = rank;
        MPI_Comm icomm; // коммуникатор после очищение но до добавления процессов
        MPI_Comm scomm; // локальный коммуникатор для каждой стороны icomm???восстановленных
        MPI_Comm mcomm; // интракомм слияние

        std::string str;

        if( comm == MPI_COMM_NULL )
        {
            MPI_Comm_get_parent( &icomm );
            scomm = MPI_COMM_WORLD;
        }
        else
        {
            console::Log( ) << console::extensions::Level::Error << "Detected failed process";
            MPIX_Comm_failure_ack( comm );
            MPI_Group group_f;
            MPIX_Comm_failure_get_acked( comm, &group_f );
            int nf;
            MPI_Group_size( group_f, &nf );
            MPI_Group group_c;
            std::vector< int > ranks_gf( nf );
            std::vector< int > ranks_gc( nf );
            MPI_Comm_group( comm, &group_c );
            for( int i = 0; i < nf; i++ )
            {
                ranks_gf[ i ] = i;
            }
            MPI_Group_translate_ranks( group_f, nf, ranks_gf.data( ), group_c, ranks_gc.data( ) );

            str += "\n\nFAILED RANKS ";

            for( int i = 0; i < nf; i++ )
            {
                str += std::string{ "  " } + std::to_string( ranks_gc[ i ] ) + std::string{ " " };
            }
            str += "\n";

            failingProc.resize( ranks_gc.size() );
            std::copy( ranks_gc.begin(), ranks_gc.end(), failingProc.begin() );


            /// 0 1 *2* 3 4 5
            /// 0 1  3  4 5 2

            for( auto &proc : failingProc )
            {
                for( std::size_t i = proc; i < virtual_ranks.size( ) - 1; i++ )
                    virtual_ranks[ i ] = virtual_ranks[ i + 1 ];

            }




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

            MPI_Info mpi_info;
            MPI_Info_create( &mpi_info );
            MPI_Info_set( mpi_info, "host", "ub002" );
            //MPI_Info_set( mpi_info, "hostfile", "hostfile" );
            //MPI_Info_set( mpi_info, "add-hostfile", "hostfile" );

            //MPI_Info_set( mpi_info, "host", "ub002" );

            console::Log( ) << "IN REPLACE: " << NetworkModule::getName( );

            //if( NetworkModule::getName( ) == "ub002" )
            //{
            //    std::this_thread::sleep_for( std::chrono::seconds{ 5 } );
            //}

            std::ofstream stream( "file" + std::to_string( NetworkModule::Instance( ).getRank( ) ) );
            stream << "nd = " << nd << "\n";

            auto rc = MPI_Comm_spawn(
                    "testapp",
                    MPI_ARGV_NULL,
                    nd,
                    mpi_info,
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
                console::Log( ) << rank << ": comm_spawn failed";
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
            console::Log( ) << rank << ": Intercomm_merge failed";
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
        str += "old rank = " + std::to_string( oldrank ) + " newRank = " + std::to_string( rank ) + "\n";
        console::Log( ) << console::extensions::Level::Error <<  str;
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
        if( __toRank == rank )
            throw exception::Attempt_to_send_to_oneself( __FILE__, __LINE__ );
        MPI_Request request;
        //while( true )
        {
            std::unique_lock< std::mutex > guard( mutex );
            auto rc = MPI_Issend(
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
                spawn( );
            }
            //else
            //    break;
        }
        return request;
    }

    MPI_Request
    NetworkModule::send(
            const std::string &__data,
            const Number __toRank
        )
    {
        if( __toRank == rank )
            throw exception::Attempt_to_send_to_oneself( __FILE__, __LINE__ );
        MPI_Request request;
        //while( true )
        {
            std::unique_lock< std::mutex > guard( mutex );
            auto rc = MPI_Issend(
                    ( void* )__data.data( ),
                    static_cast< int >( __data.size( ) ),
                    MPI_CHAR,
                    __toRank,
                    MPI_ANY_TAG,
                    world,
                    &request
                               );
            if( rc != MPI_SUCCESS )
            {
                spawn( );
            }
            //else
            //    break;
        }
        return request;
    }

    std::string
    NetworkModule::getMessage(
            MPI_Status &status
        )
    {
        std::string buf;
        //while( true )
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
                spawn( );
            }
            //else
            //    break;
        }
        return buf;
    }

    bool NetworkModule::test( NetworkModule::Request &request, NetworkModule::Status &status )
    {
        int testVar = 0;
        int rc = 0;
        //while( true )
        {
            std::unique_lock< std::mutex > guard( mutex );
            rc = MPI_Test( &request, &testVar, &status );
            if( rc != MPI_SUCCESS )
            {
                spawn( );
            }
            //else
            //    break;
        }

        if( ( testVar == 1 ) && ( rc == MPI_SUCCESS ) )
        {
            return true;
        }
        else
            return false;
    }

    void NetworkModule::cancel(NetworkModule::Request &request)
    {
        //while( true )
        {
            std::unique_lock< std::mutex > guard( mutex );
            auto rc = MPI_Cancel( &request );
            if( rc != MPI_SUCCESS )
                spawn( );
            //else
            //    break;
        }
    }

    void NetworkModule::abort( )
    {
        std::unique_lock< std::mutex > guard( mutex );
        MPI_Abort( MPI_COMM_WORLD, 0 );
    }

    std::tuple< bool, MPI_Status >
    NetworkModule::checkMessage( )
    {
        MPI_Status status;
        int messageFounded;
        //while( true )
        {
            std::unique_lock< std::mutex > guard( mutex );
            int rc = MPI_Iprobe(
                        MPI_ANY_SOURCE,
                        MPI_ANY_TAG,
                        world,
                        &messageFounded,
                        &status
                               );
            if( rc != MPI_SUCCESS )
            {
                spawn( );
            }
            //else
            //    break;
        }
        return std::make_tuple( messageFounded == 1, status );
    };

    std::tuple< bool, MPI_Status >
    NetworkModule::checkMessage( Number source, TypeMessage typeMessage )
    {
        if( source == rank )
            throw exception::Attempt_to_check_to_oneself( __FILE__, __LINE__ );

        MPI_Status status;
        int messageFounded;
        //while( true )
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
                spawn( );
            }
            //else
            //    break;
        }
        return std::make_tuple( messageFounded == 1, status );
    }

    NetworkModule::~NetworkModule( )
    {
        std::unique_lock< std::mutex > guard( mutex );
        MPI_Finalize( );
    }

    MPI_Request NetworkModule::send( const Number __toRank, const TypeMessage __typeMessage )
    {
        if( __toRank >= size  )
            throw exception::Illegal_rank( __FILE__, __LINE__ );

        MPI_Request request;
        //while( true )
        {
            std::unique_lock< std::mutex > guard( mutex );
            auto rc = MPI_Issend(
                    &empty,
                    1,
                    MPI_INT,
                    __toRank,
                    static_cast< int >( __typeMessage ),
                    world,
                    &request
            );
            if( rc != MPI_SUCCESS )
                spawn( );
            //else
            //    break;
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
        //while( true )
        {
            int testVar;
            auto rc = MPI_Test( &request, &testVar, &status );
            if( rc != MPI_SUCCESS )
                spawn( );
            //else
            //    break;

            //if( testVar != 0 )
            //    break;

            std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
            auto end = std::chrono::steady_clock::now( );
            if( std::chrono::duration_cast< std::chrono::seconds >( end - start ).count( ) > sec )
            {
                std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
                auto rc = MPI_Cancel( &request );
                if( rc != MPI_SUCCESS )
                    spawn( );
                throw exception::Error_worker_logger( __FILE__, __LINE__ );
            }
        }
    }

    bool NetworkModule::getError( )
    {
        return isReplace;
    }

    void NetworkModule::resetError( )
    {
        isReplace = false;
    }

    void NetworkModule::Abort( )
    {
        MPI_Abort( world, 10 );
    }

}



