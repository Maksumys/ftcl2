#include "log.hpp"
#include "ftcl/network.hpp"

#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>
#include <list>
#include <iterator>

namespace ftcl { namespace console {

    Logger::Logger( )
    {
        file.open( path, std::ios::app );
        #ifdef FTCL_MPI_INCLUDED
                if( NetworkModule::Instance( ).master( ) )
                    thread = new std::thread( &Logger::runMaster, this );
                else
                    thread = new std::thread( &Logger::runWorker, this );
        #else
                thread = new std::thread( &Logger::run, this );
        #endif
    }

    Logger::~Logger( )
    {
        exit = true;
        thread -> join( );
        #ifdef FTCL_MPI_INCLUDED
        {
            if( fail )
            {
                std::cout << "FAIL LOG EXIT" << std::endl;
                NetworkModule::Instance( ).abort( );
            }
        }
        #endif
    }



#ifdef FTCL_MPI_INCLUDED
    void Logger::runMaster( )
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
                if( fileEnabled )                                               \
                    file << multiQueueStream.back( );
                if( consoleEnabled )                                            \
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




    void Logger::runWorker( )
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

    bool Logger::runAllowMaster( )
    {
        return runAllow( ) || !allLoggersClosed || !empty( );
    }

    bool Logger::runAllowWorker( )
    {
        return runAllow( ) || !workerReply;
    }

    std::size_t Logger::exitMaster( )
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

#endif

    bool
    Logger::runAllow( )
    {
        return !empty( ) || !exit;
    }

    bool
    Logger::empty( )
    {

        return multiQueueStream.empty( );
    }

    void
    Logger::run( )
    {
        while( runAllow( ) )
        {
            if( !empty( ) )
            {
                if( fileEnabled )                                               \
                    file << multiQueueStream.back( );
                if( consoleEnabled )                                            \
                    std::cout << multiQueueStream.back( );
                multiQueueStream.pop( );
            }
            std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
        }
    }

    Logger&
    Logger::Instance( )
    {
        static Logger log;
        return log;
    }

    void
    Logger::enableFile( ) noexcept
    {
        fileEnabled = true;
    }

    void
    Logger::disableFile( ) noexcept
    {
        fileEnabled = false;
    }

    void
    Logger::enableConsole( ) noexcept
    {
        consoleEnabled = true;
    }

    void
    Logger::disableConsole( ) noexcept
    {
        consoleEnabled = false;
    }

    void
    Logger::enableOutputTime( ) noexcept
    {
        timeEnabled = true;
    }

    void
    Logger::disableOutputTime( ) noexcept
    {
        timeEnabled = false;
    }

    void
    Logger::setPath(const std::string &__path)
    {
        path = __path;
        if( file.is_open( ) )
        {
            file.close( );
            file.open( __path, std::ios::app );
        }
    }

    Level Logger::getLevel( )
    {
        return globalLevel;
    }

    void Logger::setLevel(const Level &__level)
    {
        globalLevel = __level;
    }

    std::string
    Logger::getCurrentTime( ) const noexcept
    {
        if( timeEnabled )
        {
            auto now = std::chrono::system_clock::now( );
            auto now_c = std::chrono::system_clock::to_time_t( now );
            std::stringstream ss;
            ss << "[ "
               << std::put_time( std::localtime( &now_c ), "%Y-%m-%d %X" )
               << " ] ";
            return ss.str( );
        }
        else
            return std::string{ };
    }

    Logger&
    operator<<( Logger &__logger, const LogMessage &__message )
    {
        if( __logger.thread == nullptr )
            throw  exception::Log_was_not_started( __FILE__, __LINE__ );

        std::string str = __message.time;
#ifdef FTCL_MPI_INCLUDED
        str += " << Node " + std::to_string( __message.numNode ) + " >> ";
#endif
        str +=  __message.color +
                __message.stream.data( ) +
                Color::RESET;

        __logger.multiQueueStream.push( str );

        return __logger;
    }




    Log& Log::operator<<( int value ) { msg.stream += std::to_string( value ); return *this; }
    Log& Log::operator<<( long value ) { msg.stream += std::to_string( value ); return *this; }
    Log& Log::operator<<( long long value ) { msg.stream += std::to_string( value ); return *this; }
    Log& Log::operator<<( unsigned int value ) { msg.stream += std::to_string( value ); return *this; }
    Log& Log::operator<<( unsigned long value ) { msg.stream += std::to_string( value ); return *this; }
    Log& Log::operator<<( unsigned long long int value ) { msg.stream += std::to_string( value ); return *this; }
    Log& Log::operator<<( float value ) { msg.stream += std::to_string( value ); return *this; }
    Log& Log::operator<<( double value ) { msg.stream += std::to_string( value ); return *this; }
    Log& Log::operator<<( const std::string &value ) { msg.stream += value; return *this; }

    Log::~Log( )
    {
        msg.stream += "\n";
        msg.time = Color::BOLDCYAN + Logger::Instance( ).getCurrentTime( ) + Color::RESET;
#ifdef FTCL_MPI_INCLUDED
        msg.numNode  = NetworkModule::Instance( ).getRank( );
#endif
        bool allow = false;
        levelToColor( allow );
        if( allow )
        {
            Logger::Instance( ) << msg;
        }
    }

    void Log::levelToColor( bool &__allow )
    {
        switch( msg.level )
        {
            case Level::Info:
            {
                if( Logger::Instance( ).getLevel( ) >= Level::Info )
                {
                    msg.color = Color::GREEN;
                    __allow = true;
                }
                break;
            }
            case Level::Warning:
            {
                if( Logger::Instance( ).getLevel( ) >= Level::Warning )
                {
                    msg.color = Color::YELLOW;
                    __allow = true;
                }
                break;
            }
            case Level::Error:
            {
                if( Logger::Instance( ).getLevel( ) >= Level::Error )
                {
                    msg.color = Color::RED;
                    __allow = true;
                }
                break;
            }
            case Level::Debug1:
            {
                if( Logger::Instance( ).getLevel( ) >= Level::Debug1 )
                {
                    msg.color = Color::WHITE;
                    __allow = true;
                }
                break;
            }
            case Level::Debug2:
            {
                if( Logger::Instance( ).getLevel( ) >= Level::Debug2 )
                {
                    msg.color = Color::BOLDWHITE;
                    __allow = true;
                }
                break;
            }
        }
    }

    Log &Log::operator<<( const Precission &__precission )
    {
        msg.delimiter = __precission.delimiter;
        return *this;
    }

    Log &Log::operator<<( Precission &__precission )
    {
        msg.delimiter = __precission.delimiter;
        return *this;
    }

    Log &Log::operator<<(Level &__level)
    {
        msg.level = __level;
        return *this;
    }

    Log &Log::operator<<(const Level &__level)
    {
        msg.level = __level;
        return *this;
    }

} }
