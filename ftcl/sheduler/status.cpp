#include "ftcl/sheduler/status.hpp"

namespace ftcl
{
    StatusWorker::StatusWorker( const std::uint64_t &__countWorkers )
    {
        totalNumberWorkers = __countWorkers;
        workingNumberWorkers = __countWorkers;
    }
    
    bool StatusWorker::isAllInitialize( )
    {
        
        return false;
    }
}
