#ifndef FTCL_SHEDULER_HPP_INCLUDED
#define FTCL_SHEDULER_HPP_INCLUDED

#include "ftcl/console/log.hpp"
#include <queue>

namespace ftcl
{
    enum class Events
    {
        GetInitializeWorker,
        GetWorkersName,
        ShutDown
    };

    class Sheduler
    {
    public:
        Sheduler( ) = default;
        virtual ~Sheduler( ) = default;

        virtual void run( ) = 0;
        //virtual void initialize( ) = 0;
        //virtual void finalize( ) = 0;

        Sheduler( Sheduler& ) = delete;
        Sheduler& operator=( const Sheduler& ) = delete;
    };
}

#endif // FTCL_SHEDULER_HPP_INCLUDED
