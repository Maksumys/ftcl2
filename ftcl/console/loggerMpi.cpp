#ifdef FTCL_MPI_INCLUDED

#include "ftcl/console/loggerMpi.hpp"
#include "ftcl/network.hpp"

#include <iterator>

namespace _ftcl::console
{
    LoggerMpi&
    LoggerMpi::Instance( )
    {
        static LoggerMpi log;
        return log;
    }

    LoggerMpi::LoggerMpi( )
    {
        if( NetworkModule::Instance( ).master( ) )
        {
            try
            {
                thread = new std::thread( &LoggerMpi::runMaster, this );
            }
            catch( std::bad_alloc& )
            {
                std::cout << "logger was not created on master Node. Abort!" << std::endl;
                NetworkModule::Instance( ).abort( );
            }
        }
        else
        {
            try
            {
                thread = new std::thread( &LoggerMpi::runWorker, this );
            }
            catch( std::bad_alloc& )
            {
                std::cout << "logger was not created on worker Node with name: \""
                          << NetworkModule::Instance( ).getName( )
                          << "\""
                          << " Abort!" << std::endl;
                NetworkModule::Instance( ).abort( );
            }
        }
    }

    LoggerMpi::~LoggerMpi( )
    {
        exit = true;
        thread -> join( );
        if( fail )
        {
            std::cout << "FAIL LOG EXIT" << std::endl;
            NetworkModule::Instance( ).abort( );
        }
    }

    void LoggerMpi::runMaster( )
    {
        if( !NetworkModule::Instance( ).master( ) )
            throw exception::Illegal_rank( __FILE__, __LINE__ );

        std::size_t sendCountExit = 0;
        bool willExited = false;
        std::chrono::steady_clock::time_point start;
        std::size_t countRecv = 0;
        while( runAllowMaster( ) )
        {
            if( !empty( ) )
            {
                if( fileEnabled )
                    file << multiQueueStream.back( );
                if( consoleEnabled )
                    std::cout << multiQueueStream.back( );
                multiQueueStream.pop( );
            }
            else
                std::this_thread::sleep_for( std::chrono::microseconds( 1 ) );

            auto check = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::MessageLog );
            if( std::get< 0 >( check ) )
            {
                auto msg = NetworkModule::Instance( ).getMessage( std::get< 1 >( check ) );
                std::copy( msg.begin( ), msg.end( ), std::ostream_iterator< char >( std::cout, "" ) );
                std::copy( msg.begin( ), msg.end( ), std::ostream_iterator< char >( file, "" ) );
            }

            if( ( exit ) && ( !willExited ) && ( empty( ) ) )
            {
                sendCountExit = exitMaster( );
                willExited = true;
                start = std::chrono::steady_clock::now( );
            }

            if( willExited )
            {
                auto check1 = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::MessageLogExit );
                auto check2 = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::MessageLog );

                if( ( std::get< 0 >( check1 ) ) && ( !std::get< 0 >( check2 ) ) )
                {
                    NetworkModule::Instance( ).getMessage( std::get< 1 >( check1 ) );
                    countRecv++;
                }

                std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
                auto end = std::chrono::steady_clock::now( );
                if( std::chrono::duration_cast< std::chrono::seconds >( end - start ).count( ) > 5 )
                {
                    fail = true;
                    allLoggersClosed = true;
                }
                if( countRecv == sendCountExit )
                    allLoggersClosed = true;
            }
        }
    }

    void LoggerMpi::runWorker( )
    {
        if( NetworkModule::Instance( ).master( ) )
            throw exception::Illegal_rank( __FILE__, __LINE__ );
        try
        {
            while( runAllowWorker( ) )
            {
                if( !empty( ) )
                {
                    NetworkModule::Request request = NetworkModule::Instance( ).send(
                            multiQueueStream.back( ), 0, TypeMessage::MessageLog
                        );

                    NetworkModule::Status status;
                    auto start = std::chrono::steady_clock::now( );
                    while( true )
                    {
                        if( NetworkModule::Instance( ).test( request, status ) )
                            break;

                        std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
                        auto end = std::chrono::steady_clock::now( );
                        if( std::chrono::duration_cast< std::chrono::seconds >( end - start ).count( ) > 5 )
                        {
                            std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
                            NetworkModule::Instance( ).cancel( request );
                            throw exception::Error_worker_logger( __FILE__, __LINE__ );
                        }
                    }
                    multiQueueStream.pop( );
                }
                else if( ( masterSendExit ) && ( exit ) )
                {
                    NetworkModule::Status status;
                    std::string str = " ";
                    NetworkModule::Request request = NetworkModule::Instance( ).send(
                           str, 0, TypeMessage::MessageLogExit
                        );

                    auto start = std::chrono::steady_clock::now( );
                    while( true )
                    {
                        if( NetworkModule::Instance( ).test( request, status ) )
                            break;

                        std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
                        auto end = std::chrono::steady_clock::now( );
                        if( std::chrono::duration_cast< std::chrono::seconds >( end - start ).count( ) > 5 )
                        {
                            std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
                            NetworkModule::Instance( ).cancel( request );
                            throw exception::Error_worker_logger( __FILE__, __LINE__ );
                        }
                    }
                    workerReply = true;
                }

                if( !masterSendExit )
                {
                    auto checkExit = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::MessageLogExit );
                    if( std::get< 0 >( checkExit ) )
                    {
                        NetworkModule::Instance( ).getMessage( std::get< 1 >( checkExit ) );
                        masterSendExit = true;
                    }
                }
            }
        }
        catch(...)
        {
            fail = true;
        }
    }

    bool LoggerMpi::runAllowMaster( )
    {
        return !empty( ) || !exit || !allLoggersClosed || !empty( );
    }

    bool LoggerMpi::runAllowWorker( )
    {
        return !empty( ) || !exit || !workerReply;
    }

    std::size_t LoggerMpi::exitMaster( )
    {
        std::string str = " ";
        for( int i = 0; i < NetworkModule::Instance( ).getSize( ) - 1; i++ )
        {
            vectorRequest.push_back( std::make_tuple( NetworkModule::Instance( ).send( str, i + 1, TypeMessage::MessageLogExit ), false ) );
        }

        std::size_t count = 0;
        auto start = std::chrono::steady_clock::now( );
        while( count < vectorRequest.size( ) )
        {
            NetworkModule::Status status;
            for( auto &elem : vectorRequest )
            {
                if( !std::get< 1 >( elem ) )
                {
                        if( NetworkModule::Instance().test( std::get< 0 >( elem ), status ) )
                        {
                            count++;
                            std::get< 1 >( elem ) = true;
                        }
                }
            }

            std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );

            auto end = std::chrono::steady_clock::now( );
            if( std::chrono::duration_cast< std::chrono::seconds >( end - start ).count( ) > 5 )
            {
                for( auto &elem : vectorRequest )
                {
                    if( !std::get< 1 >( elem ) )
                        MPI_Cancel( &std::get< 0 >( elem ) );
                }
                fail = true;
                break;
            }
        }
        return count;
    }

    LoggerMpi&
    operator<<( LoggerMpi &__logger, const extensions::LogMessage &__message )
    {
        if( __logger.thread == nullptr )
            throw  exception::Log_was_not_started( __FILE__, __LINE__ );

        std::string str = __message.time;
        //str += " << Node " + std::to_string( __message.numNode ) + " >> ";
        str +=  __message.color +
                __message.stream.data( ) +
                extensions::Color::RESET;

        __logger.multiQueueStream.push( str );

        return __logger;
    }
}
#endif
