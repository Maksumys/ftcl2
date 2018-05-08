#ifndef _FTCL_SHEDULER_STATUS_HPP
#define _FTCL_SHEDULER_STATUS_HPP

#include "ftcl/network.hpp"
#include "ftcl/sheduler/sheduler.hpp"
#include "ftcl/timer.hpp"

#include <cstdint>
#include <vector>
#include <chrono>
#include <mpi.h>
/*
namespace ftcl
{
    class _StatusWorker
    {
    public:
        State state{ State::idle };
        std::string name;
        MPI_Request workerNameRequest;
        MPI_Request workerShutDownRequest;
        bool sendedShutDown{ false };
        Timer timeCurrentState;
        bool isInit{ false };


    };

    class networkInterface
    {
    protected:
        std::vector< _StatusWorker >                    statuses;                       ///< вектор состояний ворверов
        std::size_t                                     countInitializedWorkers;        ///< количество инициализированных воркеров
        std::size_t                                     secWaitReq{ 3 };                ///< время ожидания запросов
        std::size_t                                     countShutDownWorkers{ 0 };      ///< количество завершенных воркеров
    public:
        networkInterface(  );
        explicit networkInterface( std::uint64_t __countWorkers );
    };


    class StatusWorker
    {
    public:
        std::vector< _StatusWorker > statuses;
        std::size_t countInitializedWorkers;
        std::size_t secWaitSend{ 3 };

        std::size_t countShutDownWorkers{ 0 };
        explicit StatusWorker( const std::uint64_t &__countWorkers );

        bool isInit( std::size_t __numWorkers  );
        bool recvInitialize( std::size_t __numWorkers );
        bool getWorkersName( std::size_t __numWorkers );
        bool getReqTask( std::size_t __numWorkers );

        bool shutDown( std::size_t __numWorkers );



        void sendTask( std::size_t numWorker, const std::string &str );

        template< typename _TypeTask >
        void getTask( std::size_t numWorker, _TypeTask &task );

        void printStatusWorkers( );
        std::string getWorkerName( std::size_t numWorker );
    };

}*/
#endif
