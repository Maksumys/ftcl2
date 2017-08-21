#include "log.hpp"

#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>


namespace ftcl { namespace console {

    Logger::Logger( )
    {
        file.open( path, std::ios::app );
        if( multiThreadsEnabled )
            thread = new std::thread( &Logger::runMultiThread, this );
        else
            thread = new std::thread( &Logger::run, this );
    }

    Logger::~Logger( )
    {
        exit = true;
        thread -> join( );
    }

    void Logger::runMultiThread( )
    {
        while( ( !exit ) || ( !multiQueueStream.empty( ) ) )
        {
            if( !multiQueueStream.empty( ) )
            {
                if( fileEnabled )
                    file << multiQueueStream.back( );
                if( consoleEnabled )
                    std::cout << multiQueueStream.back( );
                multiQueueStream.pop( );
            }
            else
                std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
        }
    }

    void Logger::run( )
    {
        while( ( !exit ) || ( !queueStream.empty( ) ) )
        {
            if( !queueStream.empty( ) )
            {
                if( fileEnabled )
                    file << queueStream.back( );
                if( consoleEnabled )
                    std::cout << queueStream.back( );
                queueStream.pop( );
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
        if( !multiThreadsEnabled )
        {
            exit = true;
            thread->join( );
            delete thread;
            multiThreadsEnabled = true;
            exit = false;
            thread = new std::thread( &Logger::runMultiThread, this );
        }
    }

    void Logger::disableMultiThreads( ) noexcept
    {
        if( multiThreadsEnabled )
        {
            exit = true;
            thread->join( );
            delete thread;
            multiThreadsEnabled = false;
            exit = false;
            thread = new std::thread( &Logger::runMultiThread, this );
        }
    }

    void Logger::enableOutputTime( ) noexcept
    {
        timeEnabled = true;
    }

    void Logger::disableOutputTime( ) noexcept
    {
        timeEnabled = false;
    }

    void Logger::set(const std::string &__path)
    {
        path = __path;
        if( file.is_open( ) )
        {
            file.close( );
            file.open( __path, std::ios::app );
        }
    }

    std::string Logger::getCurrentTime( ) const noexcept
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






    Logger& operator<<( Logger &logger, const std::string &str )
    {
        if( logger.multiThreadsEnabled )
            logger.multiQueueStream.push( str );
        else
            logger.queueStream.push( str );
        return logger;
    }

    Logger& operator<<( Logger &logger, const char* str )
    {   
        if( logger.multiThreadsEnabled )
            logger.multiQueueStream.push( std::string{ str } );
        else
            logger.queueStream.push( std::string{ str } );
        return logger;
    }

    Logger& endl( Logger& logger )
    {
        if( logger.multiThreadsEnabled )
            logger.multiQueueStream.push( std::string{ "\n" } );
        else
            logger.queueStream.push( std::string{ "\n" } );
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







