#ifndef FTCL_SHEDULER_MASTER_HPP_INCLUDEDD
#define FTCL_SHEDULER_MASTER_HPP_INCLUDEDD

#include "ftcl/network.hpp"
#include "ftcl/sheduler/sheduler.hpp"
#include "ftcl/sheduler/status.hpp"
#include "ftcl/exception.hpp"
#include "ftcl/timer.hpp"

#include <queue>

namespace ftcl
{
    //template< class Task >
    class ShedulerMaster : public Sheduler
    {
    protected:
        using NumWorker = std::size_t;
        StatusWorker statuses;

        //std::queue< Task > tasks;
    public:

        //void setTasks( std::queue< Task > &&_tasks )
        //{
        //    tasks = _tasks;
        //}


        explicit ShedulerMaster(
            ) :
                statuses( NetworkModule::Instance( ).getSize( ) - 1 )
        {
            using namespace console;
            Log( ) << console::extensions::Level::Debug2
                   << "Create sheduler Master";

            for( std::size_t i = 0; i < NetworkModule::Instance( ).getSize( ) - 1; i++ )
            {
                events.push( std::make_tuple( i + 1, Events::GetInitializeWorker ) );
            }

            func.emplace(
                Events::GetInitializeWorker,
                [ & ]( NumWorker numWorker )
                {
                    if( statuses.recvInitialize( numWorker ) )
                    {
                        Log( ) << "Worker " << numWorker << " initialized!";
                        events.push( std::make_tuple( numWorker, Events::GetWorkersName ) );
                    }

                    if( !statuses.isInit( numWorker ) )
                    {
                        events.push( std::make_tuple( numWorker, Events::GetInitializeWorker ) );
                    }
                    else
                    {
                        statuses.printStatusWorkers( );
                    }
                } );


            func.emplace(
                Events::GetWorkersName,
                [ & ]( NumWorker numWorker )
                {
                    if( !statuses.getWorkersName( numWorker ) )
                        events.push( std::make_tuple( numWorker, Events::GetWorkersName ) );
                    else
                    {
                        Log( ) << "Master recv worker name( "
                               << statuses.getWorkerName( numWorker )
                               << " )! NumWorker: "
                               << numWorker;
                        statuses.printStatusWorkers( );
                        events.push( std::make_tuple( numWorker, Events::ShutDown ) );
                    }
                } );

            func.emplace(
                    Events::GetTask,
                    [ & ]( NumWorker numWorker )
                    {
                        /*if( !tasks.empty( ) )
                        {
                            std::stringstream stream;
                            stream << tasks.front( );

                        }*/
                    }
                );


            func.emplace(
                Events::ShutDown,
                [ & ]( NumWorker numWorker )
                {
                    if( !statuses.shutDown( numWorker ) )
                    {
                        events.push( std::make_tuple( numWorker, Events::ShutDown ) );
                    }
                    else
                    {
                        Log( ) << "Master recv worker shutdown( "
                               << statuses.getWorkerName( numWorker )
                               << " )! NumWorker: "
                               << numWorker;
                        statuses.countShutDownWorkers++;
                        statuses.printStatusWorkers( );
                    }
                } );
        }

        void run( ) override
        {
            using namespace console;
            while( !events.empty( ) )
            {
                NumWorker numWorker;
                Events event;
                std::tie( numWorker, event ) = events.front( );
                events.pop( );
                try
                {
                    func[ event ]( numWorker );
                }
                catch( exception::Error_communication &ex )
                {
                    ex.what( );
                    statuses.statuses[ numWorker ] = _StatusWorker{ };
                    events.push( std::make_tuple( numWorker, Events::GetInitializeWorker ) );
                }
                catch( Exception &ex )
                {
                    ex.what( );
                }
            }


            if( statuses.countShutDownWorkers < NetworkModule::Instance( ).getSize( ) - 1 )
            {
                Log( ) << "ABORT!";
                NetworkModule::Instance( ).abort( );
            }
        }
    };
}

#endif
