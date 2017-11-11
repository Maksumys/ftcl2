#include "ftcl/console/loggerBase.hpp"

#include <iomanip>

namespace _ftcl::console
{
    LoggerBase::LoggerBase( )
    {
        file.open( path, std::ios::app );
    }

    LoggerBase::~LoggerBase( )
    {
        exit = true;
    }

    bool
    LoggerBase::empty( )
    {
        return multiQueueStream.empty( );
    }

    void
    LoggerBase::enableFile( ) noexcept
    {
        fileEnabled = true;
    }

    void
    LoggerBase::disableFile( ) noexcept
    {
        fileEnabled = false;
    }

    void
    LoggerBase::enableConsole( ) noexcept
    {
        consoleEnabled = true;
    }

    void
    LoggerBase::disableConsole( ) noexcept
    {
        consoleEnabled = false;
    }

    void
    LoggerBase::enableOutputTime( ) noexcept
    {
        timeEnabled = true;
    }

    void
    LoggerBase::disableOutputTime( ) noexcept
    {
        timeEnabled = false;
    }

    void
    LoggerBase::setPath(const std::string &__path)
    {
        path = __path;
        if( file.is_open( ) )
        {
            file.close( );
            file.open( __path, std::ios::app );
        }
    }

    extensions::Level LoggerBase::getLevel( )
    {
        return globalLevel;
    }

    void LoggerBase::setLevel(const extensions::Level &__level)
    {
        globalLevel = __level;
    }

    std::string
    LoggerBase::getCurrentTime( ) const noexcept
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
}
