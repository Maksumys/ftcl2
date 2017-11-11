#ifndef _FTCL_CONSOLE_LOG_HPP_INCLUDED
#define _FTCL_CONSOLE_LOG_HPP_INCLUDED

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <list>


#include "ftcl/queue.hpp"
#include "ftcl/multithread/queue.hpp"
#include "ftcl/console/logExtensions.hpp"

namespace ftcl::console
{
    class Log
    {
    protected:
        ftcl::console::extensions::LogMessage msg;
    public:
        ~Log( );
        Log& operator<<( int __value );
        Log& operator<<( long int __value );
        Log& operator<<( long long int __value );
        Log& operator<<( unsigned int __value );
        Log& operator<<( unsigned long int __value );
        Log& operator<<( unsigned long long int __value );
        Log& operator<<( float __value );
        Log& operator<<( double __value );
        Log& operator<<( const std::string &__value );

        Log& operator<<( const extensions::Level &__level );
        Log& operator<<( extensions::Level &__level );
        Log &operator<<( const extensions::Precission &precission );
        Log &operator<<( extensions::Precission &precission );

        template< class T >
        Log& operator<<( const std::vector< T > &value );

        template< class T, std::size_t size >
        Log& operator<<( const std::array< T, size > &array );

        void levelToColor( bool &__allow );

        void enableFile( ) noexcept;
        void disableFile( ) noexcept;

        void enableConsole( ) noexcept;
        void disableConsole( ) noexcept;

        void enableOutputTime( ) noexcept;
        void disableOutputTime( ) noexcept;

        void setPath( const std::string &__path );

        extensions::Level getLevel(  );
        void setLevel( const extensions::Level &__level );
    };

    template< class T >
    Log& Log::operator<<( const std::vector< T > &value )
    {
        for( const auto &elem : value )
            msg.stream += std::to_string( elem ) + msg.delimiter;
        return *this;
    }

    template< class T, std::size_t size >
    Log& Log::operator<<( const std::array< T, size > &array )
    {
        for( const auto &elem : array )
            msg.stream += std::to_string( elem ) + msg.delimiter;
        return *this;
    }
}


#endif //_FTCL_CONSOLE_LOG_HPP_INCLUDED
