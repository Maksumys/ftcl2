#ifndef _FTCL_SHEDULER_STATUS_HPP
#define _FTCL_SHEDULER_STATUS_HPP

#include "ftcl/network.hpp"

#include <cstdint>
#include <vector>
#include <chrono>
#include <mpi.h>

namespace ftcl
{
    enum class State
    {
        idle = 0,
        working,
        failing,
        shutdowning,
    };
    
    class _StatusWorker
    {
    public:
        State state;
        std::chrono::steady_clock::time_point timeCurrentState;
        MPI_Request requestWorkerName;
    };
    
    class StatusWorker
    {
    public:
        std::uint64_t totalNumberWorkers;
        std::uint64_t workingNumberWorkers;
        
        std::vector< _StatusWorker > statuses;
        
        explicit StatusWorker( const std::uint64_t &__countWorkers );
        bool isAllInitialize( );
    };

}
#endif
