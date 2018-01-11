#ifndef _FTCL_SHEDULER_STATUS_HPP
#define _FTCL_SHEDULER_STATUS_HPP

#include "ftcl/network.hpp"
#include "ftcl/sheduler/sheduler.hpp"
#include "ftcl/timer.hpp"

#include <cstdint>
#include <vector>
#include <chrono>
#include <mpi.h>

namespace ftcl
{
    enum class State
    {
        idle = 0,
        initialize,
        waitingName,
        waitingTask,
        working,
        failing,
        readyToShutDown,
        shutdowning,
    };

    class _StatusWorker
    {
    public:
        State state;
        std::string name;
        MPI_Request workerNameRequest;
        MPI_Request workerShutDownRequest;
        bool sendedShutDown{ false };
        Timer timeCurrentState;
    };

    class StatusWorker
    {
    public:
        std::vector< _StatusWorker > statuses;
        std::size_t countInitializedWorkers;
        std::size_t secWaitSend{ 3 };

        std::size_t countShutDownWorkers{ 0 };
        
        explicit StatusWorker( const std::uint64_t &__countWorkers );
        bool recvInitialize( const std::size_t __numWorkers );
        bool getWorkersName( const std::size_t __numWorkers );
        void printStatusWorkers( );
        bool shutDown( const std::size_t __numWorkers );
        bool isAllInitialize( );
        std::string getWorkerName( std::size_t numWorker );
    };

}
#endif
