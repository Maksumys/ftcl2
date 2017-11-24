#ifndef _FTCL_SHEDULER_WORKER_HPP_INCLUDED
#define _FTCL_SHEDULER_WORKER_HPP_INCLUDED

#include "ftcl/sheduler/sheduler.hpp"
#include "ftcl/network.hpp"

namespace ftcl
{
    class ShedulerWorker : public Sheduler
    {
    private:
        std::string data;
    public:
        ShedulerWorker( )
        {
            console::Log( ) << console::extensions::Level::Debug2
                            << "Create sheduler Worker";
        }

        void run( ) override
        {
            console::Log( ) << console::extensions::Level::Debug2 << "Run sheduler Worker " << NetworkModule::Instance( ).getName( );
            while( true )
            {
                auto check = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::requestWorkersName );
                if (std::get< 0 >( check ) )
                {
                    console::Log( ) << console::extensions::Level::Debug2
                                   << "Worker recv request worker name";
                    auto msg = NetworkModule::Instance( ).getMessage( std::get< 1 >( check ) );

                    auto request = NetworkModule::Instance( ).send(
                            data,
                            0,
                            TypeMessage::MessageWorkerName
                        );
                    console::Log( ) << console::extensions::Level::Debug2
                                    << "Worker send name";
                    NetworkModule::Status status;
                    NetworkModule::Instance( ).wait( request, status, 3 );
                    break;
                }
            }
            //std::this_thread::sleep_for( std::chrono::seconds{ 10 } );
            /*check = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::MessageShutdownMasterToWorker );
            if( std::get< 0 >( check ) )
            {
                NetworkModule::Instance().getMessage( std::get< 1 >( check ) );
                NetworkModule::Instance( ).send( NetworkModule::Instance( ).getName( ), 0, TypeMessage::MessageShutdownWorkerToMaster );
            }*/
            console::Log( ) << console::extensions::Level::Debug2 << "exit run sheduler Worker";
        }
        void initialize( ) override { }
        void finalize( ) override { }
        ~ShedulerWorker( )
        {
            //MPI_Status status;
            //MPI_Wait( &request, &status );
        }
    };
}

#endif
