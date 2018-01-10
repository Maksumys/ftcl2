#ifndef FTCL_NETWORK_HPP_INCLUDED
#define FTCL_NETWORK_HPP_INCLUDED


/*!
 *  \brief Модуль передачи данных
 *   не правильно работает, если инициализировать в другом singleton'е
 *   при определенных условиях
 */

#include "ftcl/exception.hpp"
#include "ftcl/sheduler/status.hpp"

#include <cstdint>
#include <mpi.h>
#include <mpi-ext.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <mutex>

namespace ftcl
{
    /*!
     * \brief The TypeMessage enum Типы сообщения для модуля коммуникаций
     */
    enum class TypeMessage
    {
        WorkerInitialize,                   ///< сообщение инициализации воркера
        MessageLog,                         ///< сообщение логгера
        MessageLogExit,                     ///< сообщение команды завершения
        requestWorkersName,                 ///< сообщение запроса имени воркера
        MessageWorkerName,                  ///< сообщение имени воркера
        MessageShutdownMasterToWorker,      ///< сообщение команды завершения
        MessageShutdownWorkerToMaster       ///< сообщение успешного завершения
    };

    /*!
     * \brief The NetworkModule class Модуль коммуникаций между узлами
     * \code
     * //Пример инициализации:
     * NetworkModule::Instance( );
     *
     * //Пример передачи сообщений:
     * auto request = NetworkModule::Instance( ).send( "Hello", 1, TypeMessage::MessageLog );
     * MPI_Status status;
     * NetworkModule::Instance( ).wait( request, status, 5 );
     */
    class NetworkModule
    {
    public:
        using Number = int;
        using Request = MPI_Request;
        using Status = MPI_Status;
    protected:
        Number              rank;                       ///< номер текущего процесса
        Number              size;                       ///< общее количество процессов
        std::string         name;                       ///< имя узла на котором запущен процесс
        mutable std::mutex  mutex;                      ///< синхронизация вызовов функций
        int                 empty = 0;                  ///< костыль для пустого сообщения
        char** gargv;
        MPI_Comm world;
        MPI_Comm rworld;
        NetworkModule( );
    public:
        /*!
         * \brief Instance Инициализация модуля
         * \return Статический объект модуля
         */
        static NetworkModule& Instance( );

        void Spawn( );
        void initialize( int argc, char **argv );

        int MPIX_Comm_replace( MPI_Comm comm, MPI_Comm *newcomm );
        /*!
         * \brief getSize Возвращает количество процессов
         * \return Кол-во процессов
         */
        Number getSize( ) const noexcept;

        /*!
         * \brief getRank Возвращает текущий номер процесса
         * \return Текущий номер процесса
         */
        Number getRank( ) const noexcept;

        /*!
         * \brief master Возвращает признак главного процесса
         * \return true если текущий процесс главный
         */
        bool master( ) const noexcept;

        /*!
         * \brief send Неблокирующая отправка сообщения с данными
         * \param __data Данные для отправки
         * \param __toRank Номер процесса которому отправляется сообщение
         * \param __typeMessage Тип сообщения
         * \return Идентификатор операции отправки
         */
        MPI_Request
        send(
                const std::string   &__data,
                const Number        __toRank,
                const TypeMessage   __typeMessage
            );

        /*!
         * \brief send Неблокирующая отправка сообщения без данных
         * \param __toRank Номер процесса которому отправляется сообщение
         * \param __typeMessage Тип сообщения
         * \return Идентификатор операции отправки
         */
        MPI_Request
        send(
                const Number        __toRank,
                const TypeMessage   __typeMessage
        );

        /*!
         * \brief checkMessage Проверка входящих сообщений
         * \param source ( -1 значит MPI_ANY_SOURCE )
         * \param typeMessage Тип сообщения
         * \return < true если есть входящее сообщение, данные входящего сообщения >
         */
        std::tuple< bool, MPI_Status >
        checkMessage(
                const Number source,
                const TypeMessage typeMessage
            );

        /*!
         * \brief getMessage Взятие входящего сообщения
         * \param status Данные входящего сообщения
         * \return Сообщение
         */
        std::vector< char >
        getMessage(
                MPI_Status &status
            );

        std::string getName( );

        bool test( NetworkModule::Request &request, NetworkModule::Status &status );
        void cancel( NetworkModule::Request &request );
        void abort( );

        void wait( MPI_Request &request, MPI_Status &status, const std::int64_t sec );
        //void wait( const StatusWorker &state );
        MPI_Comm getParentComm( );

        static void verbose_errhandler( MPI_Comm *comm, int *perr, ... );

    private:
        ~NetworkModule( );
    };

}

#endif // FTCL_NETWORK_HPP_INCLUDED
