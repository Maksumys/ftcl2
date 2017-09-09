#ifndef _FTCL_CONSOLE_LOG_HPP_INCLUDED
#define _FTCL_CONSOLE_LOG_HPP_INCLUDED

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>

#include "ftcl/queue.hpp"
#include "ftcl/multithread/queue.hpp"

#ifdef FTCL_MPI_INCLUDED
    #include <mpi.h>
#endif

namespace ftcl { namespace console {

    #ifdef FTCL_MPI_INCLUDED
        #define RUNLOGGER                                               \
            if( NetworkModule::Instance( ).master( ) )                  \
            {                                                           \
                thread = new std::thread( &Logger::runMaster, this );   \
            }                                                           \
            else                                                        \
            {                                                           \
                thread = new std::thread( &Logger::runWorker, this );   \
            }
    #else
        #define RUNLOGGER                                               \
            thread = new std::thread( &Logger::run, this );
    #endif
    #define WAITLOGGER                                                  \
        thread -> join( );

    #define FILE_OUTPUT                                                 \
        if( fileEnabled )                                               \
        {                                                               \
            if( logMode == LogMode::singleThread )                      \
                file << queueStream.back( );                            \
            else                                                        \
                file << multiQueueStream.back( );                       \
        }

    #define CONSOLE_OUTPUT                                              \
        if( consoleEnabled )                                            \
        {                                                               \
            if( logMode == LogMode::singleThread )                      \
                std::cout << queueStream.back( );                       \
            else                                                        \
                std::cout << multiQueueStream.back( );                  \
        }


    #define MESSAGE_POP                                                 \
        if( logMode == LogMode::singleThread )                          \
        {                                                               \
            queueStream.pop( );                                         \
        }                                                               \
        else                                                            \
            multiQueueStream.pop( );

    #define MESSAGE_PUSH                                                \
        if( __logger.logMode == LogMode::multiThread )                  \
            __logger.multiQueueStream.push( str );                      \
        else                                                            \
        {                                                               \
            __logger.queueStream.push( str );                           \
        }

    enum class LogMode
    {
        singleThread, multiThread
    };

#ifdef FTCL_MPI_INCLUDED
    enum class MpiMode
    {
        mpiMaster, mpiWorker
    };
#endif

    namespace Color
    {
        constexpr auto RESET = "\033[0m";
        constexpr auto BLACK = "\033[30m";              /// Black
        constexpr auto RED = "\033[31m";                /// Red
        constexpr auto GREEN = "\033[32m";              /// Green
        constexpr auto YELLOW = "\033[33m";             /// Yellow
        constexpr auto BLUE = "\033[34m";               /// Blue
        constexpr auto MAGENTA = "\033[35m";            /// Magenta
        constexpr auto CYAN = "\033[36m";               /// Cyan
        constexpr auto WHITE = "\033[37m";              /// White
        constexpr auto BOLDBLACK = "\033[1m\033[30m";   /// Bold Black
        constexpr auto BOLDRED = "\033[1m\033[31m";     /// Bold Red
        constexpr auto BOLDGREEN = "\033[1m\033[32m";   /// Bold Green
        constexpr auto BOLDYELLOW = "\033[1m\033[33m";  /// Bold Yellow
        constexpr auto BOLDBLUE = "\033[1m\033[34m";    /// Bold Blue
        constexpr auto BOLDMAGENTA = "\033[1m\033[35m"; /// Bold Magenta
        constexpr auto BOLDCYAN = "\033[1m\033[36m";    /// Bold Cyan
        constexpr auto BOLDWHITE = "\033[1m\033[37m";   /// Bold White
    }

    enum class Level
    {
        Error, Info, Warning, Debug1, Debug2
    };

    class Precission
    {
    public:
        Precission( const std::string &__delimiter ) : delimiter( __delimiter ) { }
        std::string delimiter { "" };
    };

    class LogMessage
    {
    public:
        Level level{ Level::Info };
        std::string time;
        std::string color;
        std::string stream;
        std::string delimiter;

#ifdef FTCL_MPI_INCLUDED
        std::size_t numNode;
#endif

    };

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

        void enableOutputTime( ) noexcept;
        void disableOutputTime( ) noexcept;

        void setPath( const std::string &__path );

        Level getLevel(  );
        void setLevel( const Level &__level );

        std::string getCurrentTime( ) const noexcept;

    protected:

        /// settings
        Level globalLevel{ Level::Info };
        bool fileEnabled{ true };
        bool consoleEnabled{ true };
        bool exit{ false };
        bool timeEnabled{ true };
        bool fail{ false };

        LogMode logMode{ LogMode::singleThread };

        std::ofstream file;
        std::string path{ "log.txt" };

        queue< std::string > queueStream{ 100 };
        multithread::queue< std::string > multiQueueStream{ 100 };

        std::thread *thread;

        Logger( );
        ~Logger( );



#ifdef FTCL_MPI_INCLUDED
        /*!
         * \brief run Запуск логгера (MPI)
         */
        void runMaster( );
        void runWorker( );

        bool runAllowMaster( );
        bool runAllowWorker( );


        bool allLoggersClosed { false };        //< Признак, что все логгеры закрыты на узлах
        bool masterSendExit { false };          //< Мастер прислал сигнал завершения

#endif
        void run( );







        /*!
         * \brief runAllow Проверка на выход из цикла вывода сообщений
         * \return true если сообщения позволено выводить
         */
        bool runAllow( );

        /*!
         * \brief empty Проверка очередей на элементы
         * \return
         */
        bool empty( );

        friend Logger& operator<<( Logger &__logger, const LogMessage &__msg );
    };

    Logger& operator<<( Logger &__logger, const std::string &__str );
    Logger& operator<<( Logger &__logger, const char* __str );

    class Log
    {
    protected:
        LogMessage msg;
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

        Log& operator<<( const Level &__level );
        Log& operator<<( Level &__level );
        Log &operator<<( const Precission &precission );
        Log &operator<<( Precission &precission );

        template< class T >
        Log& operator<<( const std::vector< T > &value );

        template< class T, std::size_t size >
        Log& operator<<( const std::array< T, size > &array );

        void levelToColor( bool &__allow );
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

} }

#endif //_FTCL_CONSOLE_LOG_HPP_INCLUDED
