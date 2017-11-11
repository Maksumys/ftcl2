#ifndef FTCL_MPI_INCLUDED
#ifndef _FTCL_CONSOLE_LOGGER_COMMON_HPP_INCLUDED
#define _FTCL_CONSOLE_LOGGER_COMMON_HPP_INCLUDED

#include "ftcl/console/logExtensions.hpp"
#include "ftcl/console/loggerBase.hpp"

namespace _ftcl::console
{
    class LoggerCommon : public LoggerBase
    {
    public:
        static LoggerCommon& Instance( );
        LoggerCommon( const LoggerCommon& ) = delete;
        LoggerCommon( LoggerCommon&& ) = delete;
        LoggerCommon& operator=( const LoggerCommon& ) = delete;
        LoggerCommon& operator=( LoggerCommon&& ) = delete;

    protected:
        LoggerCommon( );
        ~LoggerCommon( );

        /*!
         * \brief run Запуск логгера (обычный режим)
         */
        void run( );

        /*!
         * \brief runAllow Проверка на выход из цикла вывода сообщений
         * \return true если сообщения позволено выводить
         */
        bool runAllow( );
        friend LoggerCommon& operator<<( LoggerCommon &__logger, const extensions::LogMessage &__msg );
    };
     LoggerCommon& operator<<( LoggerCommon &__logger, const extensions::LogMessage &__msg );
}

#endif //_FTCL_CONSOLE_LOGGER_COMMON_HPP_INCLUDED
#endif