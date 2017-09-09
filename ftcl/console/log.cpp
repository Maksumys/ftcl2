#include "log.hpp"
#include "ftcl/network.hpp"

#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>
#include <list>

namespace ftcl { namespace console {

    Logger::Logger( )
    {
        file.open( path, std::ios::app );
        RUNLOGGER
    }

    Logger::~Logger( )
    {
        exit = true;
        WAITLOGGER
        #ifdef FTCL_MPI_INCLUDED
        if( fail )
        {
            std::cout << "FAIL LOG EXIT" << std::endl;
            MPI_Abort( MPI_COMM_WORLD, 0 );
        }
        #endif
    }


#ifdef FTCL_MPI_INCLUDED
    void Logger::runMaster( )
    {
        if( !NetworkModule::Instance( ).master( ) )
            throw exception::Illegal_rank( __FILE__, __LINE__ );

        while( runAllowMaster( ) )
        {
            if( !empty( ) )
            {
                FILE_OUTPUT
                CONSOLE_OUTPUT
                MESSAGE_POP

                auto check = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::MessageLog );
                if( std::get< 0 >( check ) )
                {
                    auto msg = NetworkModule::Instance( ).getMessage( std::get< 1 >( check ) );
                    std::copy( msg.begin( ), msg.end( ), std::ostream_iterator< char >( std::cout, "" ) );
                }
            }

            if( exit )
            {
                std::list< MPI_Request > vectorRequest;
                for( int i = 0; i < NetworkModule::Instance( ).getSize( ) - 1; i++ )
                {
                    vectorRequest.push_back( NetworkModule::Instance( ).send( " ", i, TypeMessage::MessageLogExit ) );
                }

                std::size_t count = 0;
                auto start = std::chrono::steady_clock::now( );
                while( count < vectorRequest.size( ) )
                {
                    MPI_Status status;
                    int flag;
                    std::list< MPI_Request >::iterator it;
                    while( it != vectorRequest.end( ) )
                    {
                        MPI_Test( &*it, &flag, &status );
                        if( flag )
                        {
                            count++;
                            it = vectorRequest.erase( it );
                        }
                    }
                    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

                    auto end = std::chrono::steady_clock::now( );
                    if( std::chrono::duration_cast< std::chrono::seconds >( end - start ).count( ) > 5 )
                    {
                        for( auto &elem : vectorRequest )
                            MPI_Cancel( &elem );

                        fail = true;
                        break;
                    }
                }

                std::size_t countRecv = 0;
                start = std::chrono::steady_clock::now( );

                while( countRecv < count )
                {
                    auto check = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::MessageLogExit );
                    if( std::get< 0 >( check ) )
                    {
                        NetworkModule::Instance( ).getMessage( std::get< 1 >( check ) );
                        countRecv++;
                    }
                    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
                    auto end = std::chrono::steady_clock::now( );
                    if( std::chrono::duration_cast< std::chrono::seconds >( end - start ).count( ) > 5 )
                    {
                        fail = true;
                        allLoggersClosed = true;
                    }
                }
                allLoggersClosed = true;
            }
            std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
        }
    }

    void Logger::runWorker()
    {
        if( NetworkModule::Instance( ).master( ) )
            throw exception::Illegal_rank( __FILE__, __LINE__ );
        try
        {
            while( runAllowWorker( ) )
            {
                if( !empty( ) )
                {
                    MPI_Request request;
                    if( logMode == LogMode::singleThread )
                        request = NetworkModule::Instance( ).send(
                                queueStream.back( ), 0, TypeMessage::MessageLog
                            );
                    else
                        request = NetworkModule::Instance( ).send(
                                multiQueueStream.back( ), 0, TypeMessage::MessageLog
                            );

                    MPI_Status status;
                    int flag;
                    auto start = std::chrono::steady_clock::now( );
                    while( true )
                    {
                        MPI_Test( &request, &flag, &status );
                        if( flag == 1 )
                        {
                            break;
                        }
                        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
                        auto end = std::chrono::steady_clock::now( );
                        if( std::chrono::duration_cast< std::chrono::seconds >( end - start ).count( ) > 5 )
                        {
                            std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
                            MPI_Cancel( &request );
                            throw exception::Error_worker_logger( __FILE__, __LINE__ );
                        }
                    }
                    MESSAGE_POP
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
        return runAllow( ) && !allLoggersClosed;
    }

    bool Logger::runAllowWorker( )
    {
        return runAllow( ) && !masterSendExit;
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
        if( logMode == LogMode::singleThread )
            return queueStream.empty( );
        else
            return multiQueueStream.empty( );
    }

    void
    Logger::run( )
    {
        while( runAllow( ) )
        {
            if( !empty( ) )
            {
                FILE_OUTPUT
                CONSOLE_OUTPUT
                MESSAGE_POP
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
    Logger::enableMultiThreads( ) noexcept
    {
        if( logMode == LogMode::singleThread )
        {
            exit = true;
            thread->join( );
            delete thread;
            logMode = LogMode::singleThread;
            exit = false;
            thread = new std::thread( &Logger::run, this );
        }
    }

    void
    Logger::disableMultiThreads( ) noexcept
    {
        if( logMode == LogMode::multiThread )
        {
            exit = true;
            thread->join( );
            delete thread;
            logMode = LogMode::singleThread;
            exit = false;
            thread = new std::thread( &Logger::run, this );
        }
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
        std::string str = __message.time;
#ifdef FTCL_MPI_INCLUDED
        str += " << Node " + std::to_string( __message.numNode ) + " >> ";
#endif
        str +=  __message.color +
                __message.stream.data( ) +
                Color::RESET;

        MESSAGE_PUSH

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
