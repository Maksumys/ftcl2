#include "log.hpp"

#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>


namespace ftcl { namespace console {

    Logger::Logger( )
    {
        thread = new std::thread( &Logger::run, this );
    }

    Logger::~Logger( )
    {
        exit = true;
        thread -> join( );
    }

    void Logger::run( )
    {
        while( ( !exit ) || ( !multiQueueStream.empty( ) ) )
        {
            if( !multiQueueStream.empty( ) )
            {
                std::cout << multiQueueStream.back( );
                multiQueueStream.pop( );
            }
            else
                std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
        }
    }

    Logger& Logger::Instance( )
    {
        static Logger log;
        return log;
    }

    void Logger::enableFile( ) noexcept
    {
        fileEnabled = true;
    }

    void Logger::disableFile( ) noexcept
    {
        fileEnabled = false;
    }

    void Logger::enableConsole( ) noexcept
    {
        consoleEnabled = true;
    }

    void Logger::disableConsole( ) noexcept
    {
        consoleEnabled = false;
    }

    void Logger::enableMultiThreads( ) noexcept
    {
        multiThreadsEnabled = true;
    }

    void Logger::disableMultiThreads( ) noexcept
    {
        multiThreadsEnabled = false;
    }

    std::string Logger::getCurrentTime( ) const noexcept
    {
        auto now = std::chrono::system_clock::now( );
        auto now_c = std::chrono::system_clock::to_time_t( now );
        std::stringstream ss;
        ss << "[ "
           << std::put_time( std::localtime( &now_c ), "%Y-%m-%d %X" )
           << " ] ";
        return ss.str( );
    }






    Logger& operator<<( Logger &logger, const std::string &str )
    {
        logger.multiQueueStream.push( str );
        return logger;
    }

    Logger& operator<<( Logger &logger, const char* str )
    {
        logger.multiQueueStream.push( std::string{ str } );
        return logger;
    }

    Logger& endl( Logger& logger )
    {
        logger.multiQueueStream.push( std::string{ "\n" } );
        return logger;
    }




    inline Log& Log::operator<<( int value ) { stream << value; return *this; }
    inline Log& Log::operator<<( long value ) { stream << value; return *this; }
    inline Log& Log::operator<<( long long value ) { stream << value; return *this; }
    inline Log& Log::operator<<( unsigned int value ) { stream << value; return *this; }
    inline Log& Log::operator<<( unsigned long value ) { stream << value; return *this; }
    inline Log& Log::operator<<( unsigned long long value ) { stream << value; return *this; }
    inline Log& Log::operator<<( float value ) { stream << value; return *this; }
    inline Log& Log::operator<<( double value ) { stream << value; return *this; }
    inline Log& Log::operator<<( std::string &value ) { stream << value; return *this; }
    Log& Log::operator<<( const char *value ) { stream << value; return *this; }

    Log::~Log( )
    {
        stream << std::endl;
        Logger::Instance( ) <<  Logger::Instance( ).getCurrentTime( ) + stream.str( );
    }










} }







