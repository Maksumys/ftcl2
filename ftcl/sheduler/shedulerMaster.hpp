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
    template< typename _TypeTask >
    class ShedulerMaster : public Sheduler
    {
    protected:
        using NumWorker = std::size_t;
        StatusWorker                        statuses;           ///< статусы воркеров и действия над ними
        std::queue< _TypeTask >             tasks;              ///< очередь задач
    public:
        explicit ShedulerMaster(
            ) :
                statuses( NetworkModule::Instance( ).getSize( ) - 1 )
        {
            using namespace console;
            Log( ) << console::extensions::Level::Debug2
                   << "Create sheduler Master";

            for( std::size_t i = 0; i < NetworkModule::Instance( ).getSize( ) - 1; i++ )
                events.emplace( i + 1, Events::GetInitializeWorker );

            func.emplace(
                Events::GetInitializeWorker,
                [ & ]( NumWorker numWorker )
                {
                    if( statuses.recvInitialize( numWorker ) )
                        events.emplace( numWorker, Events::GetWorkersName );

                    if( !statuses.isInit( numWorker ) )
                        events.emplace( numWorker, Events::GetInitializeWorker );
                    else
                        statuses.printStatusWorkers( );
                } );


            func.emplace(
                Events::GetWorkersName,
                [ & ]( NumWorker numWorker )
                {
                    if( !statuses.getWorkersName( numWorker ) )
                        events.emplace( numWorker, Events::GetWorkersName );
                    else
                    {
                        Log( ) << "Master recv worker name( "
                               << statuses.getWorkerName( numWorker )
                               << " )! NumWorker: "
                               << numWorker;
                        statuses.printStatusWorkers( );
                        events.emplace( numWorker, Events::GetTask );
                    }
                } );

            func.emplace(
                    Events::GetTask,
                    [ & ]( NumWorker numWorker )
                    {
                        if( !tasks.empty( ) )
                        {
                            statuses.getTask( numWorker, tasks.front( ) );
                        }
                    }
                );


            func.emplace(
                Events::ShutDown,
                [ & ]( NumWorker numWorker )
                {
                    if( !statuses.shutDown( numWorker ) )
                        events.emplace( numWorker, Events::ShutDown );
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

        void repair( )
        {
            using namespace console;
            auto failingProc = NetworkModule::Instance( ).getFailingProc( );

            for( auto &proc : failingProc )
            {
                auto countEvents = events.size( );
                for( std::size_t i = 0; i < countEvents; i++ )
                {
                    std::size_t numWorker;
                    Events event;
                    std::tie( numWorker, event ) = events.front( );
                    if( numWorker > proc )
                        events.emplace( numWorker - 1, event );
                    else if( numWorker < proc )
                        events.emplace( numWorker, event );
                    events.pop( );
                }

                for( std::size_t i = 0; i < failingProc.size( ); i++ )
                    events.emplace( NetworkModule::Instance( ).getSize( ) - i - 1, Events::GetInitializeWorker );

                for( std::size_t i = proc; i < statuses.statuses.size( ); i++ )
                    statuses.statuses[ i - 1 ] = statuses.statuses[ i ];
            }

            for( std::size_t i = 0; i < failingProc.size( ); i++ )
            {
                statuses.statuses[ statuses.statuses.size( ) - 1 - i ] = _StatusWorker{ };
            }
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
                catch( Exception &ex )
                {
                    ex.what( );
                }

                if( NetworkModule::Instance( ).getError( ) )
                {
                    repair( );
                    NetworkModule::Instance( ).resetError( );
                }
            }

            statuses.printStatusWorkers( );

            if( statuses.countShutDownWorkers < NetworkModule::Instance( ).getSize( ) - 1 )
            {
                Log( ) << "ABORT!";
                std::this_thread::sleep_for( std::chrono::seconds{ 5 } );
                NetworkModule::Instance( ).abort( );
            }
        }
    };
}

#endif
