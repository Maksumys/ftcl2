#ifdef FTCL_MPI_INCLUDED
#ifndef _FTCL_CONSOLE_LOGGER_MPI_INCLUDED
#define _FTCL_CONSOLE_LOGGER_MPI_INCLUDED

#include "ftcl/console/loggerBase.hpp"
#include "ftcl/console/logExtensions.hpp"

#include <mpi.h>

namespace _ftcl::console
{
    using namespace ftcl::console;

    enum class MpiMode
    {
        mpiMaster, mpiWorker
    };

    class LoggerMpi : public LoggerBase
    {
    public:
        static LoggerMpi& Instance( );
        LoggerMpi( const LoggerMpi& ) = delete;
        LoggerMpi( LoggerMpi&& ) = delete;
        LoggerMpi& operator=( const LoggerMpi& ) = delete;
        LoggerMpi& operator=( LoggerMpi&& ) = delete;

    protected:
        LoggerMpi( );
        ~LoggerMpi( );

        /*!
         * \brief run Запуск логгера на мастере (MPI)
         */
        void runMaster( );
        /*!
         * \brief run Запуск логгера на воркере (MPI)
         */
        void runWorker( );

        /*!
         * \brief runAllowMaster условие продолжения работы на мастере
         * \return true, если нужно продолжить работу
         */
        bool runAllowMaster( );

        /*!
         * \brief runAllowWorker условие продолжения работы на воркере
         * \return true, если нужно продолжить работу
         */
        bool runAllowWorker( );

        std::size_t exitMaster( );

        bool allLoggersClosed { false };        //< Признак, что все логгеры закрыты на узлах
        bool masterSendExit { false };          //< Мастер прислал сигнал завершения
        bool workerReply { false };             //< Воркер отправил на мастера что закрывается

        std::list< std::tuple< MPI_Request, bool > > vectorRequest;

        friend LoggerMpi&
        operator<<( LoggerMpi &__logger, const extensions::LogMessage &__message );
    };

    LoggerMpi&
    operator<<( LoggerMpi &__logger, const extensions::LogMessage &__message );
}

#endif // _FTCL_CONSOLE_LOGGER_MPI_INCLUDED
#endif // FTCL_MPI_INCLUDED
