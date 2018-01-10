#ifndef _FTCL_SHEDULER_WORKER_HPP_INCLUDED
#define _FTCL_SHEDULER_WORKER_HPP_INCLUDED

#include "ftcl/sheduler/sheduler.hpp"
#include "ftcl/network.hpp"
#include "ftcl/timer.hpp"

#include <queue>

namespace ftcl
{
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
        }

        void run( ) override
        {
            using namespace console;

            NetworkModule::Request reqWorkInit;
            
            std::size_t timeWorkInit{ 10000 };
            Timer timer;
            timer.start( );
            bool sendedWorkInit{ false };

            while( !events.empty( ) )
            {
                auto[ num, event ] = events.front( );
                switch( event )
                {
                    case Events::GetInitializeWorker:
                    {
                        if( !sendedWorkInit )
                        {
                            reqWorkInit = NetworkModule::Instance( ).send( num, TypeMessage::WorkerInitialize );
                            sendedWorkInit = true;
                        }

                        NetworkModule::Status status;
                        auto test = NetworkModule::Instance( ).test( reqWorkInit, status );
                        if( test )
                            break;

                        if( timer.end( ) < timeWorkInit )
                            events.push( std::make_tuple( num, event ) );
                        else
                        {
                            NetworkModule::Instance( ).cancel( reqWorkInit );
                            sendedWorkInit = false;                                                        
                            Log( ) << "Worker don't send init to master!";
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
