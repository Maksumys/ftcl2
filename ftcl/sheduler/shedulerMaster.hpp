#ifndef FTCL_SHEDULER_MASTER_HPP_INCLUDED
#define FTCL_SHEDULER_MASTER_HPP_INCLUDED

#include "ftcl/network.hpp"
#include "ftcl/sheduler/sheduler.hpp"
#include "ftcl/sheduler/status.hpp"
#include "ftcl/exception.hpp"
#include "ftcl/timer.hpp"

#include <queue>

namespace ftcl
{
    /*!
     *
     */
    class ShedulerMaster : public Sheduler
    {
    protected:
        using NumWorker = std::size_t;

        std::int64_t maxSecTime;
        StatusWorker statuses;
        std::int64_t secWaitRequests{ 3 };
        std::queue< std::tuple< NumWorker, Events > > events;
    public:
        ShedulerMaster(
                std::uint64_t __countWorkers,
                std::int64_t __maxSecTime
            ) :
                maxSecTime( __maxSecTime ),
                statuses( __countWorkers )
        {
            using namespace console;
            Log( ) << console::extensions::Level::Debug2
                   << "Create sheduler Master";
            maxSecTime = __maxSecTime;

            for( std::size_t i = 0; i < __countWorkers; i++ )
            {
                events.push( std::make_tuple( i + 1, Events::GetInitializeWorker ) );
            }
        }

        void run( ) override
        {
            using namespace console;
            bool init = false;
            while( !events.empty( ) )
            {
                auto[ numWorker, event ] = events.front( );
                switch( event )
                {
                    case Events::GetInitializeWorker:
                    {
                        try
                        {
                            if( statuses.recvInitialize( numWorker ) )
                            {
                                Log( ) << "Worker " << numWorker << " initialized!";
                                events.push( std::make_tuple( numWorker, Events::GetWorkersName ) );
                                init = true;
                            }
                        }
                        catch( Exception &e )
                        {
                            Log( ) << e.what( );
                        }

                        if( !init )
                        {
                            events.push( std::make_tuple( numWorker, event ) );
                        }
                        else
                        {
                            statuses.printStatusWorkers( );
                        }
                        break;
                    }
                    case Events::GetWorkersName:
                    {
                        try
                        {
                            if( !statuses.getWorkersName( numWorker ) )
                                events.push( std::make_tuple( numWorker, event ) );
                            else
                            {
                                Log( ) << "Master recv worker name( "
                                       << statuses.getWorkerName( numWorker )
                                       << " )! NumWorker: "
                                       << numWorker;
                                events.push( std::make_tuple( numWorker, Events::ShutDown ) );
                            }
                        }
                        catch( Exception &e )
                        {
                            Log( ) << e.what( );
                        }
                        break;
                    }
                    case Events::ShutDown:
                    {
                        try
                        {
                            if( !statuses.shutDown( numWorker ) )
                            {
                                events.push( std::make_tuple( numWorker, event ) );
                            }
                            else
                            {
                                Log( ) << "Master recv worker shutdown( "
                                       << statuses.getWorkerName( numWorker )
                                       << " )! NumWorker: "
                                       << numWorker;
                                statuses.countShutDownWorkers++;
                            }
                        }
                        catch( Exception &e )
                        {
                            Log( ) << e.what( );
                        }
                        break;
                    }
                }
                events.pop( );
            }

            if( statuses.countShutDownWorkers < NetworkModule::Instance().getSize( ) )
            {
                Log( ) << "ABORT!";
                NetworkModule::Instance( ).abort( );
            }
        }

        /*void initialize( ) override
        {
            std::uint64_t countInitializing = 0;
            std::uint64_t numState = 0;

            /// Отправка запроса на имена воркеров
            while( numState < statuses.totalNumberWorkers )
            {
                console::Log( ) << console::extensions::Level::Debug2
                                << "master send request to rank "
                                << numState + 1;
                statuses.statuses[ numState ].requestWorkerName = NetworkModule::Instance( ).send( numState + 1, TypeMessage::requestWorkersName );
                numState++;
            }

            /// Ожидание ответа имен воркеров
            numState = 0;
            std::int64_t countWorkersReq = 0;
            Timer timer;
            timer.start( );
            while( timer.end( ) < 3000 )
            {
                numState = 0;
                while( numState < statuses.totalNumberWorkers )
                {
                    auto [ checkRequestName, requestName ] = 
                        NetworkModule::Instance( ).checkMessage( 
                                -1,
                                TypeMessage::MessageWorkerName
                            );
                    if( checkRequestName )
                    {
                        auto msg = NetworkModule::Instance().getMessage( requestName );
                        console::Log( ) << console::extensions::Level::Debug2
                                        << "master recv worker name";
                        countWorkersReq++;
                    }
                    numState++;
                }
                if( countWorkersReq == statuses.totalNumberWorkers )
                {
                    break;
                }
            }
            console::Log( ) << console::extensions::Level::Debug2
                            << "master success initialized";
        }

        void finalize( ) override
        {
            std::uint64_t numState = 0;
            while( numState < statuses.totalNumberWorkers )
            {
                NetworkModule::Instance( ).send( numState + 1, TypeMessage::MessageShutdownMasterToWorker );
                console::Log( ) << console::extensions::Level::Debug2
                                << "Master send worker shutdown";
                numState++;
            }
            numState = 0;
            std::uint64_t countRecv = 0;
            while( countRecv < statuses.totalNumberWorkers )
            {
                auto [ checkRequestShutdown, requestShutdown ] = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::MessageShutdownWorkerToMaster );
                if( checkRequestShutdown )
                {
                    auto msg = NetworkModule::Instance().getMessage( requestShutdown );
                    console::Log( ) << console::extensions::Level::Debug2
                                    << "Master recv shutdown";
                    countRecv++;
                }
            }
            console::Log( ) << console::extensions::Level::Debug2
                            << "master success finalize";
        }*/
    };
}

#endif
