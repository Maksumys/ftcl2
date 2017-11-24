#ifndef _FTCL_SHEDULER_MASTER_HPP_INCLUDED
#define _FTCL_SHEDULER_MASTER_HPP_INCLUDED

#include <ftcl/network.hpp>
#include "ftcl/sheduler/sheduler.hpp"
#include "ftcl/sheduler/status.hpp"

namespace ftcl
{
    /*!
     * Timer
     *
     *
     */
    class Timer
    {
    protected:
        std::chrono::steady_clock::time_point startTime;
    public:
        void start( )
        {
            startTime = std::chrono::steady_clock::now( );
        }

        std::int64_t end( )
        {
            auto endTime = std::chrono::steady_clock::now( );
            return std::chrono::duration_cast< std::chrono::milliseconds >( endTime - startTime ).count( );
        }
    };

    class ShedulerMaster : public Sheduler
    {
    protected:
        std::int64_t maxSecTime;
        StatusWorker statuses;

        std::int64_t secWaitRequests{ 3 };


    public:
        ShedulerMaster( std::uint64_t __countWorkers, std::int64_t __maxSecTime ) :
                maxSecTime( __maxSecTime ),
                statuses( __countWorkers )
        {
            console::Log( ) << console::extensions::Level::Debug2
                            << "Create sheduler Master";
            maxSecTime = __maxSecTime;
        }

        void run( ) override
        {
            try
            {
                console::Log( ) << console::extensions::Level::Info << "------------ Step 1 Initialize ------------";
                initialize( );
            }
            catch( ... )
            {
                console::Log( ) << console::extensions::Level::Debug2 << "Workers bad initialization";
            }
            console::Log( ) << console::extensions::Level::Debug2 << "exit Run sheduler master";
        }

        void initialize( ) override
        {
            std::uint64_t countInitializing = 0;

            std::uint64_t numState = 0;

            /// Отправка запроса на имена воркеров
            while( numState < statuses.totalNumberWorkers )
            {
                console::Log( ) << console::extensions::Level::Debug2 << "master send request to rank " << numState + 1;
                statuses.statuses[ numState ].requestWorkerName = NetworkModule::Instance( ).send( numState + 1, TypeMessage::requestWorkersName );
                numState++;
            }


            Timer timer;
            timer.start( );
            /// Ожидание ответа имен воркеров и проверка отправки запроса на имена ворвкеров
            while( timer.end( ) < secWaitRequests )
            {
                numState = 0;
                while( numState < statuses.totalNumberWorkers )
                {

                    numState++;
                }
            }


            /// Ожидание ответа имен воркеров
            numState = 0;
            std::int64_t countWorkersReq = 0;
            timer.start( );
            while( timer.end( ) < 3 )
            {
                numState = 0;
                while( numState < statuses.totalNumberWorkers )
                {
                    auto check = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::MessageShutdownWorkerToMaster );
                    if( std::get< 0 >( check ) )
                    {
                        auto msg = NetworkModule::Instance().getMessage( std::get< 1 >( check ) );

                        countWorkersReq++;
                    }
                    numState++;
                }
            }


            console::Log( ) << console::extensions::Level::Debug2 << "master success initialized";
        }

        void finalize( ) override
        {
            std::uint64_t numState = 0;
            /// Отправка запроса на имена ворвкеров
            while( numState < statuses.totalNumberWorkers )
            {
                NetworkModule::Instance( ).send( numState, TypeMessage::MessageShutdownMasterToWorker );
                console::Log( ) << console::extensions::Level::Debug2
                                << "Master send worker shutdown";
                numState++;
            }
            std::this_thread::sleep_for( std::chrono::seconds{ 2 } );
            numState = 0;
            std::uint64_t countRecv = 0;
            while( countRecv < statuses.totalNumberWorkers )
            {
                auto check = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::MessageShutdownWorkerToMaster );
                if( std::get< 0 >( check ) )
                {
                    auto msg = NetworkModule::Instance().getMessage( std::get< 1 >( check ) );
                    console::Log( ) << console::extensions::Level::Debug2
                                    << "Master recv shutdown";
                    countRecv++;
                }
            }
            std::this_thread::sleep_for( std::chrono::seconds{ 5 } );
        }
    };
}

#endif
