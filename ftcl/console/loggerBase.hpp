#ifndef _FTCL_CONSOLE_LOGGER_BASE_HPP_INCLUDED
#define _FTCL_CONSOLE_LOGGER_BASE_HPP_INCLUDED

#include "ftcl/console/logExtensions.hpp"

namespace _ftcl::console
{
    using namespace ftcl::console;
    using namespace ftcl;

    /*!
     * @brief The LoggerBase class является базовым классом логгера
     * @details Базовый класс логгера реализует общие свойства и методы для логгера и логгера mpi
     */
    class LoggerBase
    {
    public:
        LoggerBase( const LoggerBase& ) = delete;
        LoggerBase( LoggerBase&& ) = delete;
        LoggerBase& operator=( const LoggerBase& ) = delete;
        LoggerBase& operator=( LoggerBase&& ) = delete;

        void enableFile( ) noexcept;
        void disableFile( ) noexcept;

        void enableConsole( ) noexcept;
        void disableConsole( ) noexcept;

        void enableOutputTime( ) noexcept;
        void disableOutputTime( ) noexcept;

        void setPath( const std::string &__path );

        extensions::Level getLevel(  );
        void setLevel( const extensions::Level &__level );

        std::string getCurrentTime( ) const noexcept;

    protected:
        LoggerBase( );
        virtual ~LoggerBase( );

        /*!
         * \brief empty Проверка очередей на элементы
         * \return
         */
        bool empty( );

        /// settings
        extensions::Level globalLevel{ extensions::Level::Info };
        bool fileEnabled{ true };
        bool consoleEnabled{ true };
        bool exit{ false };
        bool timeEnabled{ true };
        bool fail{ false };

        std::ofstream file;
        std::string path{ "log.txt" };

        multithread::queue< std::string > multiQueueStream{ 100 };

        std::thread *thread = nullptr;
    };
}

#endif // _FTCL_CONSOLE_LOGGER_BASE_HPP_INCLUDED
