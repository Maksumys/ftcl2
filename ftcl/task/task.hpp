//
// Created by Максим Кузин on 27.01.2018.
//

#ifndef FTCL_TASK_HPP
#define FTCL_TASK_HPP

#include <queue>
#include "ftcl/network.hpp"
#include "ftcl/sheduler/shedulerMaster.hpp"
#include "ftcl/sheduler/shedulerWorker.hpp"

namespace ftcl
{
    class Task
    {
        virtual void run( ) = 0;
    };

    template< class Task >
    class parallel_queue
    {
    protected:
        std::queue< Task > tasks;

        Sheduler *sheduler;
    public:
        void setSheduler( Sheduler* __sheduler )
        {
            sheduler = __sheduler;
        }

        void exec( )
        {
            if( NetworkModule::Instance( ).master( ) )
                sheduler.setTasks( tasks );
            sheduler -> run( );
        }

        ~parallel_queue( )
        {
            delete sheduler;
        }
    };

    parallel_queue create_parallel_queue( int argc, char **argv )
    {
        NetworkModule::Instance( ).initialize( argc, argv );
        Sheduler *sheduler;
        if( NetworkModule::Instance( ).getRank( ) == 0 )
        {
            sheduler = new ShedulerMaster;
        }
        else
        {
            sheduler = new ShedulerWorker;
        }
        parallel_queue queue;
        queue.setSheduler( sheduler );
        return queue;
    }
}

#endif //FTCL_TASK_HPP
