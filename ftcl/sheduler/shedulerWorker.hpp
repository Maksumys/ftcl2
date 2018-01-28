#ifndef FTCL_SHEDULER_WORKER_HPP_INCLUDED
#define FTCL_SHEDULER_WORKER_HPP_INCLUDED

#include "ftcl/sheduler/sheduler.hpp"
#include "ftcl/network.hpp"
#include "ftcl/timer.hpp"

#include <queue>

namespace ftcl
{
    enum class WorkerState
    {
        idle,
        reqworking,
        working,
        shutDown,
        fail
    };

    class ShedulerWorker : public Sheduler
    {
    protected:
        using NumWorker = std::size_t;

        NetworkModule::Request reqWorkInit;
        bool sendedWorkInit{ false };
        Timer timerWorkInit;

        NetworkModule::Request reqWorkName;
        bool sendedWorkName{ false };
        Timer timerWorkName;

        NetworkModule::Request reqShutDown;
        bool sendedShutDown{ false };

        bool sendedReqTask{ false };

        std::size_t timeWorkInit{ 10000 };

    public:
        ShedulerWorker( )
        {
            console::Log( ) << console::extensions::Level::Debug2
                            << "Create sheduler Worker";
            events.emplace( 0, Events::GetInitializeWorker );
            events.emplace( 0, Events::ShutDown );

            func.emplace(
                Events::GetInitializeWorker,
                [ & ]( NumWorker num )
                {
                    using namespace console;

                    if( !sendedWorkInit )
                    {
                        timerWorkInit.start( );
                        reqWorkInit = NetworkModule::Instance( ).send( num, TypeMessage::WorkerInitialize );
                        sendedWorkInit = true;
                    }

                    NetworkModule::Status status;
                    auto test = NetworkModule::Instance( ).test( reqWorkInit, status );
                    if( test )
                    {
                        Log( ) << "worker sended initialize";
                        events.emplace( 0, Events::GetWorkersName );
                        return;
                    }

                    if( timerWorkInit.end( ) < timeWorkInit )
                    {
                        events.push( std::make_tuple( num, Events::GetInitializeWorker ) );
                    }
                    else
                    {
                        NetworkModule::Instance( ).cancel( reqWorkInit );
                        sendedWorkInit = false;
                        Log( ) << "Worker don't send init to master!";
                    }
                } );

            func.emplace(
                 Events::GetWorkersName,
                 [ & ]( NumWorker num )
                 {
                     using namespace console;
                     if( !sendedWorkName )
                     {
                         bool check;
                         NetworkModule::Status status;
                         std::tie( check, status ) = NetworkModule::Instance( ).checkMessage( 0, TypeMessage::MessageWorkerName );
                         if( check )
                         {
                             NetworkModule::Instance( ).getMessage( status );
                             timerWorkName.start( );
                             reqWorkName = NetworkModule::Instance( ).send(
                                     NetworkModule::Instance( ).getName( ),
                                     0,
                                     TypeMessage::requestWorkersName
                                  );
                              sendedWorkName = true;
                         }
                         events.push( std::make_tuple( num, Events::GetWorkersName ) );
                     }
                     else
                     {
                         NetworkModule::Status status;
                         auto test = NetworkModule::Instance( ).test( reqWorkName, status );
                         if( test )
                         {
                             events.push( std::make_tuple( num, Events::GetTask ) );
                             return;
                         }

                         if( timerWorkName.end( ) < timeWorkInit )
                             events.push( std::make_tuple( num, Events::GetWorkersName ) );
                         else
                         {
                             NetworkModule::Instance( ).cancel( reqWorkName );
                             sendedWorkName = false;
                             Log( ) << "Worker don't send name to master!";
                         }
                     }
                 } );

            func.emplace(
                        Events::GetTask,
                        [ & ]( NumWorker num )
                        {
                            if( !sendedReqTask )
                            {
                                bool check;
                                NetworkModule::Status status;
                                std::tie( check, status ) = NetworkModule::Instance( ).checkMessage( 0, TypeMessage::MessageTaskResponse );
                            }
                        }
                        );


            func.emplace(
                Events::ShutDown,
                [ & ]( NumWorker num )
                {
                    using namespace console;

                    if( !sendedShutDown )
                    {
                        bool check;
                        NetworkModule::Status status;
                        std::tie( check, status ) = NetworkModule::Instance( ).checkMessage( 0,
                                                                                             TypeMessage::MessageShutdownMasterToWorker
                                                                                           );
                        if ( check )
                        {
                            NetworkModule::Instance( ).getMessage( status );
                            Log( ) << "Worker recv shutdown";
                            std::queue< std::tuple< NumWorker, Events > > q;
                            events.swap( q );
                            timerWorkName.start( );
                            reqShutDown = NetworkModule::Instance( ).send(
                                    NetworkModule::Instance( ).getName( ),
                                    0,
                                    TypeMessage::MessageShutdownWorkerToMaster
                                                                         );
                            sendedShutDown = true;
                        }
                        events.push( std::make_tuple( num, Events::ShutDown ) );
                    }
                    else
                    {
                        NetworkModule::Status status;
                        auto test = NetworkModule::Instance( ).test( reqShutDown, status );
                        if( test )
                        {
                            return;
                        }

                        if( timerWorkName.end( ) < timeWorkInit )
                            events.push( std::make_tuple( num, Events::ShutDown ) );
                        else
                        {
                            NetworkModule::Instance( ).cancel( reqShutDown );
                            sendedShutDown = false;
                            Log( ) << "Worker don't send shutDown request to master!";
                        }
                    }
                } );
        }

        void run( ) override
        {
            using namespace console;
            while ( !events.empty( ) )
            {
                NumWorker numWorker;
                Events event;
                std::tie( numWorker, event ) = events.front( );
                events.pop( );
                func[ event ]( numWorker );
            }
        }

        ~ShedulerWorker( )
        {
        }
    };
}

#endif
