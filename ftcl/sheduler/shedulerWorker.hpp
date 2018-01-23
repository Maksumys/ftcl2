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
        shutDown,
        fail
    };

    class ShedulerWorker : public Sheduler
    {
    protected:
        using NumWorker = std::size_t;

        std::queue< std::tuple< NumWorker, Events > > events;
    public:
        ShedulerWorker( )
        {
            console::Log( ) << console::extensions::Level::Debug2
                            << "Create sheduler Worker";
            events.push( std::make_tuple( 0, Events::GetInitializeWorker ) );
            events.push( std::make_tuple( 0, Events::GetWorkersName ) );
            events.push( std::make_tuple( 0, Events::ShutDown ) );
        }

        void run( ) override
        {
            using namespace console;

            NetworkModule::Request reqWorkInit;
            bool sendedWorkInit{ false };
            Timer timerWorkInit;

            NetworkModule::Request reqWorkName;
            bool sendedWorkName{ false };
            Timer timerWorkName;


            std::size_t timeWorkInit{ 10000 };


            while( !events.empty( ) )
            {
                auto[ num, event ] = events.front( );
                switch( event )
                {
                    case Events::GetInitializeWorker:
                    {
                        if( !sendedWorkInit )
                        {
                            timerWorkInit.start( );
                            reqWorkInit = NetworkModule::Instance( ).send( num, TypeMessage::WorkerInitialize );
                            sendedWorkInit = true;
                        }

                        NetworkModule::Status status;
                        auto test = NetworkModule::Instance( ).test( reqWorkInit, status );
                        if( test )
                            break;

                        if( timerWorkInit.end( ) < timeWorkInit )
                            events.push( std::make_tuple( num, event ) );
                        else
                        {
                            NetworkModule::Instance( ).cancel( reqWorkInit );
                            sendedWorkInit = false;                                                        
                            Log( ) << "Worker don't send init to master!";
                        }
                        break;
                    }
                    case Events::GetWorkersName:
                    {
                        if( !sendedWorkName )
                        {
                            auto[ check, status ] = NetworkModule::Instance( ).checkMessage( 0, TypeMessage::MessageWorkerName );
                            if( check )
                            {
                                NetworkModule::Instance( ).getMessage( status );
                                timerWorkName.start( );
                                reqWorkName = NetworkModule::Instance( ).send(
                                        NetworkModule::Instance( ).getName( ),
                                        0,
                                        TypeMessage::MessageWorkerName
                                     );
                                sendedWorkName = true;
                            }
                        }
                        else
                        {
                            NetworkModule::Status status;
                            auto test = NetworkModule::Instance( ).test( reqWorkName, status );
                            if( test )
                                break;

                            if( timerWorkName.end( ) < timeWorkInit )
                                events.push( std::make_tuple( num, event ) );
                            else
                            {
                                NetworkModule::Instance( ).cancel( reqWorkName );
                                sendedWorkName = false;
                                Log( ) << "Worker don't send name to master!";
                            }
                        }
                        break;
                    }
                    case Events::ShutDown:
                    {
                        auto[ check, status ] = NetworkModule::Instance( ).checkMessage( 0, TypeMessage::MessageShutdownMasterToWorker );
                        if( check )
                        {
                            NetworkModule::Instance( ).getMessage( status );

                        }
                        break;
                    }
                }
                events.pop( );
            }
        }
        /* 
            MPI_Request requestSendName;
            if( NetworkModule::Instance( ).getRank( ) == 1 )
            {
                std::cout << "QWERTY!" << std::endl;
                if( NetworkModule::Instance( ).getParentComm( ) == MPI_COMM_NULL )
                {
                    std::cout << "KILL!" << std::endl;
                    std::this_thread::sleep_for( std::chrono::seconds{ 20 } );
                }
            }

            while( true )
            {
                auto [ checkRequestName, requestName ] = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::requestWorkersName );
                auto [ checkRequestShutdown, requestShutdown ] = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::MessageShutdownMasterToWorker );

                if( checkRequestName )
                {
                    console::Log( ) << console::extensions::Level::Debug2
                                    << "Worker recv request worker name";
                    auto msg = NetworkModule::Instance( ).getMessage( requestName );
                    auto request2 = NetworkModule::Instance( ).send(
                            NetworkModule::Instance( ).getName( ),
                            0,
                            TypeMessage::MessageWorkerName
                        );
                    console::Log( ) << console::extensions::Level::Debug2
                                    << "Worker send name";
                    NetworkModule::Status status;
                    NetworkModule::Instance( ).wait( request2, status, 3 );
                }
                if( checkRequestShutdown )
                {
                    NetworkModule::Instance().getMessage( requestShutdown );
                    NetworkModule::Instance( ).send(
                        0,
                        TypeMessage::MessageShutdownWorkerToMaster
                    );
                    console::Log( ) << console::extensions::Level::Debug2
                                    << "Worker recv Shutdown";
                    break;
                }
            }
            console::Log( ) << console::extensions::Level::Debug2
                            << "exit run sheduler Worker";
        }
        */
        //void initialize( ) override { }
        //void finalize( ) override { }
        ~ShedulerWorker( )
        {
        }
    };
}

#endif
