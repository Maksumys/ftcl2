#ifndef FTCL_NETWORK_HPP_INCLUDED
#define FTCL_NETWORK_HPP_INCLUDED
/*!
 *  \brief Модуль передачи данных
 *   не правильно работает, если инициализировать в другом singleton'е
 *   при определенных условиях
 */

#include "ftcl/exception.hpp"

#include <cstdint>
#include <mpi.h>
#include <mpi-ext.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <mutex>
#include <map>
#include <thread>
#include <chrono>

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
        MessageShutdownWorkerToMaster,      ///< сообщение успешного завершения
        MessageReqTask,                     ///< сообщение запроса задачи
        MessageTask,                        ///< сообщение задача
        MessageTaskResponse,                ///< сообщение ответа задачи
        MessageShutDownForce,
    };

    /*!
     * @brief The NetworkModule class Модуль коммуникаций между узлами
     * @code
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
        Number                                  rank;                       ///< номер текущего процесса
        Number                                  size;                       ///< общее количество процессов
        std::string                             name;                       ///< имя узла на котором запущен процесс
        mutable std::mutex                      mutex;                      ///< синхронизация вызовов функций
        int                                     empty = 0;                  ///< костыль для пустого сообщения
        char**                                  gargv;                      ///< аргументы командной строки
        MPI_Comm                                world;                      ///< текущий коммуникатор
        MPI_Comm                                rworld;                     ///< коммуникатор для замены в случае отказа

        std::map< std::size_t, std::size_t >    virtual_ranks;              ///< таблица виртуальных номер процессов

        bool                                    isReplace{ false };         ///< признак отказа процессов
        std::vector< std::size_t >              failingProc;                ///< номера отказавших процессов

        NetworkModule( );
    public:
        bool getError( );
        void resetError( );
        std::vector< std::size_t > getFailingProc( );

        /*!
         * \brief Instance Инициализация модуля
         * \return Статический объект модуля
         */
        static NetworkModule& Instance( );

        void spawn( );
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

        MPI_Request
        send(
                const std::string &__data,
                const Number __toRank
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

        std::tuple< bool, MPI_Status >
        checkMessage(
            );


        /*!
         * \brief getMessage Взятие входящего сообщения
         * \param status Данные входящего сообщения
         * \return Сообщение
         */
        std::string
        getMessage(
                MPI_Status &status
            );

        std::string getName( );

        bool test( NetworkModule::Request &request, NetworkModule::Status &status );
        void cancel( NetworkModule::Request &request );
        void abort( );

        void wait( MPI_Request &request, MPI_Status &status, const std::int64_t sec );
        void Abort( );
    private:
        ~NetworkModule( );
    };
}

#endif // FTCL_NETWORK_HPP_INCLUDED
