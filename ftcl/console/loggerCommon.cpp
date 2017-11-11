#ifndef FTCL_MPI_INCLUDED
#include "ftcl/console/loggerCommon.hpp"

namespace _ftcl::console
{
    LoggerCommon::LoggerCommon( )
    {
        thread = new std::thread( &LoggerCommon::run, this );
    }

    LoggerCommon::~LoggerCommon( )
    {
        exit = true;
        thread -> join( );
    }

    bool
    LoggerCommon::runAllow( )
    {
        return !empty( ) || !exit;
    }

    void
    LoggerCommon::run( )
    {
        while( runAllow( ) )
        {
            if( !empty( ) )
            {
                if( fileEnabled )
                    file << multiQueueStream.back( );
                if( consoleEnabled )
                    std::cout << multiQueueStream.back( );
                multiQueueStream.pop( );
            }
            std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
        }
    }

    LoggerCommon&
    LoggerCommon::Instance( )
    {
        static LoggerCommon log;
        return log;
    }

    LoggerCommon&
    operator<<( LoggerCommon &__logger, const extensions::LogMessage &__message )
    {
        if( __logger.thread == nullptr )
            throw  exception::Log_was_not_started( __FILE__, __LINE__ );

        std::string str = __message.time;
        str +=  __message.color +
                __message.stream.data( ) +
                extensions::Color::RESET;

        __logger.multiQueueStream.push( str );

        return __logger;
    }
}
#endif