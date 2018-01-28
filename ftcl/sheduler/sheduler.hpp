#ifndef FTCL_SHEDULER_HPP_INCLUDED
#define FTCL_SHEDULER_HPP_INCLUDED

#include "ftcl/console/log.hpp"
#include <queue>
#include <functional>

namespace ftcl
{
    enum class Events
    {
        GetInitializeWorker,
        GetWorkersName,
        GetTask,
        ShutDown
    };

    class Sheduler
    {
    protected:
        using NumWorker = std::size_t;

        std::queue< std::tuple< NumWorker, Events > > events;
        std::map< Events, std::function< void( NumWorker ) > > func;
    public:
        Sheduler( ) = default;
        virtual ~Sheduler( ) = default;

        virtual void run( ) = 0;

        Sheduler( Sheduler& ) = delete;
        Sheduler& operator=( const Sheduler& ) = delete;
    };
}

#endif // FTCL_SHEDULER_HPP_INCLUDED
