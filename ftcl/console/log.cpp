#include "ftcl/console/log.hpp"
#include "ftcl/network.hpp"
#include "ftcl/console/logger.hpp"

#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>
#include <list>
#include <iterator>

namespace ftcl::console
{
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
        if( !msg.stream.empty( ) )
        {
            msg.stream += "\n";
            msg.time = extensions::Color::BOLDCYAN +
                       Logger::Instance( ).getCurrentTime( ) +
                       extensions::Color::RESET;
            #ifdef FTCL_MPI_INCLUDED
                //msg.numNode  = NetworkModule::Instance( ).getRank( );
            #endif
            bool allow = false;
            levelToColor( allow );
            if( allow )
            {

                ftcl::console::Logger::Instance( ) << msg;
            }
        }
    }

    void Log::levelToColor( bool &__allow )
    {
        switch( msg.level )
        {
            case extensions::Level::Info:
            {
                if( Logger::Instance( ).getLevel( ) >= extensions::Level::Info )
                {
                    msg.color = extensions::Color::GREEN;
                    __allow = true;
                }
                break;
            }
            case extensions::Level::Warning:
            {
                if( Logger::Instance( ).getLevel( ) >= extensions::Level::Warning )
                {
                    msg.color = extensions::Color::YELLOW;
                    __allow = true;
                }
                break;
            }
            case extensions::Level::Error:
            {
                if( Logger::Instance( ).getLevel( ) >= extensions::Level::Error )
                {
                    msg.color = extensions::Color::RED;
                    __allow = true;
                }
                break;
            }
            case extensions::Level::Debug1:
            {
                if( Logger::Instance( ).getLevel( ) >= extensions::Level::Debug1 )
                {
                    msg.color = extensions::Color::WHITE;
                    __allow = true;
                }
                break;
            }
            case extensions::Level::Debug2:
            {
                if( Logger::Instance( ).getLevel( ) >= extensions::Level::Debug2 )
                {
                    msg.color = extensions::Color::BOLDWHITE;
                    __allow = true;
                }
                break;
            }
        }
    }

    Log &Log::operator<<( const extensions::Precission &__precission )
    {
        msg.delimiter = __precission.delimiter;
        return *this;
    }

    Log &Log::operator<<( extensions::Precission &__precission )
    {
        msg.delimiter = __precission.delimiter;
        return *this;
    }

    Log &Log::operator<<( extensions::Level &__level )
    {
        msg.level = __level;
        return *this;
    }

    Log &Log::operator<<( const extensions::Level &__level )
    {
        msg.level = __level;
        return *this;
    }

    void
    Log::enableFile( ) noexcept
    {
        Logger::Instance( ).enableFile( );
    }

    void
    Log::disableFile( ) noexcept
    {
        Logger::Instance( ).disableFile( );
    }

    void
    Log::enableConsole( ) noexcept
    {
        Logger::Instance( ).enableConsole( );
    }

    void
    Log::disableConsole( ) noexcept
    {
        Logger::Instance( ).disableConsole( );
    }

    void
    Log::enableOutputTime( ) noexcept
    {
        Logger::Instance( ).enableOutputTime( );
    }

    void
    Log::disableOutputTime( ) noexcept
    {
        Logger::Instance( ).disableOutputTime( );
    }

    void
    Log::setPath(const std::string &__path)
    {
        Logger::Instance( ).setPath( __path );
    }

    extensions::Level Log::getLevel( )
    {
        return Logger::Instance( ).getLevel( );
    }

    void Log::setLevel( const extensions::Level &__level )
    {
        Logger::Instance( ).setLevel( __level );
    }
}

