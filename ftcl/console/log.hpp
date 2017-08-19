#ifndef _FTCL_CONSOLE_LOG_HPP_INCLUDED
#define _FTCL_CONSOLE_LOG_HPP_INCLUDED

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>

#include "ftcl/queue.hpp"
#include "ftcl/multithread/queue.hpp"

namespace ftcl { namespace console {

    class Logger
    {
    public:
        static Logger& Instance( );
        Logger( const Logger& ) = delete;
        Logger( Logger&& ) = delete;
        Logger& operator=( const Logger& ) = delete;
        Logger& operator=( Logger&& ) = delete;

        void enableFile( ) noexcept;
        void disableFile( ) noexcept;

        void enableConsole( ) noexcept;
        void disableConsole( ) noexcept;

        void enableMultiThreads( ) noexcept;
        void disableMultiThreads( ) noexcept;

        std::string getCurrentTime( ) const noexcept;

    protected:

        bool fileEnabled = true;
        bool consoleEnabled = true;
        bool multiThreadsEnabled = false;
        bool exit = false;

        std::ofstream file;
        std::ostringstream console;

        queue< std::string > queueStream{ 100 };
        multithread::queue< std::string > multiQueueStream{ 100 };
        std::thread *thread;

        Logger( );
        ~Logger( );

        void run( );

        friend inline Logger& operator<<( Logger &logger, const std::string &str );
        friend inline Logger& operator<<( Logger &logger, const char* str );
        friend inline Logger& endl( Logger& logger );
    };


    class Log
    {
    protected:
        std::ostringstream stream;
    public:
        ~Log( );
        inline Log& operator<<( int value );
        inline Log& operator<<( long int value );
        inline Log& operator<<( long long int value );
        inline Log& operator<<( unsigned int value );
        inline Log& operator<<( unsigned long int value );
        inline Log& operator<<( unsigned long long value );
        inline Log& operator<<( float value );
        inline Log& operator<<( double value );
        inline Log& operator<<( std::string &value );
        Log& operator<<( const char *value );

        template< class T >
        inline Log& operator<<( const std::vector< T > &value );

        template< class T, std::size_t size >
        inline Log& operator<<( const std::array< T, size > &array );

    };

    template< class T >
    inline Log& Log::operator<<( const std::vector< T > &value )
    {
        for( const auto &elem : value )
            stream << elem << " ";
        return *this;
    }

    template< class T, std::size_t size >
    inline Log& Log::operator<<( const std::array< T, size > &array )
    {
        for( const auto &elem : array )
            stream << elem << " ";
        return *this;
    }

} }

#endif //_FTCL_CONSOLE_LOG_HPP_INCLUDED
