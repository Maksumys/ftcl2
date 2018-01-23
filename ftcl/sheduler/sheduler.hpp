#ifndef FTCL_SHEDULER_HPP_INCLUDED
#define FTCL_SHEDULER_HPP_INCLUDED

#include "ftcl/console/log.hpp"
#include "ftcl/message_manager.hpp"
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
    protected:
        messageManager manager;
    public:
        Sheduler( ) = default;
        virtual ~Sheduler( ) = default;

        virtual void run( ) = 0;
        void setMessageManager( messageManager __manager )
        {
            manager = __manager;
        }

        Sheduler( Sheduler& ) = delete;
        Sheduler& operator=( const Sheduler& ) = delete;
    };
}

#endif // FTCL_SHEDULER_HPP_INCLUDED
