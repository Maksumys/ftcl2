#ifndef FTCL_SHEDULER_WORKER_HPP_INCLUDED
#define FTCL_SHEDULER_WORKER_HPP_INCLUDED

#include "ftcl/sheduler/sheduler.hpp"
#include "ftcl/network.hpp"
#include "ftcl/timer.hpp"
#include "ftcl/checkpoint.hpp"

#include <queue>
#include <condition_variable>
#include <mutex>
#include <type_traits>
#include <experimental/optional>
#include <chrono>

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

    template< typename _TypeTask >
    class ShedulerWorker : public Sheduler< _TypeTask >
    {
        //ftcl_context< _TypeTask > context;
        std::map< Events, std::function< void( ftcl_context< _TypeTask >&, std::size_t ) > > func;

        using Sheduler<_TypeTask>::events;

    public:
        ShedulerWorker( )
        {
            using namespace console;
            using namespace console::extensions;

            console::Log( ) << "Create sheduler Worker";

            events.emplace( 0, Events::GetInitializeWorker );
            events.emplace( 0, Events::ShutDown );


            /*!
             * 1. Отправка сообщения об инициализации
             */
            func.emplace(
                Events::GetInitializeWorker,
                [ & ]( ftcl_context< _TypeTask > &context, std::size_t num )
                {
                    if( !context.request_init.sendTimedMessage( num, TypeMessage::WorkerInitialize ) )
                        events.emplace( num, Events::GetInitializeWorker );
                    else
                    {
                        events.emplace( num, Events::CheckWorkersName );
                        context.request_init.reset( );
                    }
                } );

            /*!
             * 2. Проверка запроса мастера для получения имени воркера
             */
            func.emplace(
                 Events::CheckWorkersName,
                 [ & ]( ftcl_context< _TypeTask > &context, std::size_t num )
                 {
                     bool check;
                     NetworkModule::Status status;
                     std::tie( check, status ) = NetworkModule::Instance( ).checkMessage(
                             num,
                             TypeMessage::MessageWorkerName
                        );
                     if ( check )
                     {
                         NetworkModule::Instance( ).getMessage( status );
                         events.emplace( num, Events::GetWorkersName );
                     }
                     else
                         events.emplace( num, Events::CheckWorkersName );

                 } );

            /// 3. Отправка имени воркера на мастера
            func.emplace(
                Events::GetWorkersName,
                [ & ]( ftcl_context< _TypeTask > &context, std::size_t num )
                {
                    ///TODO:
                    //std::this_thread::sleep_for( std::chrono::seconds{ 15 } );
                    if( !context.request_name.sendTimedMessage( num, TypeMessage::requestWorkersName, NetworkModule::Instance( ).getName( ) ) )
                        events.emplace( num, Events::GetWorkersName );
                    else
                    {
                        events.emplace( num, Events::GetReqTask );
                        context.request_name.reset( );
                        context.request_gettask.reset( );
                    }
                } );

            /// 4. Отправка запроса на задачу
            func.emplace(
                Events::GetReqTask,
                [ & ]( ftcl_context< _TypeTask > &context, std::size_t num )
                {
                    if( !context.request_gettask.sendTimedMessage( num, TypeMessage::MessageReqTask ) )
                        events.emplace( num, Events::GetReqTask );
                    else
                    {
                        events.emplace( num, Events::GetTask );
                        context.request_gettask.reset( );
                    }
                } );

            /// 5. Получение задачи
            func.emplace(
                    Events::GetTask,
                    [ & ]( ftcl_context< _TypeTask > &context, std::size_t num )
                    {
                        bool check;
                        NetworkModule::Status status;
                        std::tie( check, status ) = NetworkModule::Instance( ).checkMessage( num, TypeMessage::MessageTask );
                        if( check )
                        {
                            context.serialized_task = NetworkModule::Instance( ).getMessage( status );
                            _TypeTask task;
                            out::Stream stream( context.serialized_task );
                            stream >> task;
                            context.thread_task.setTask( std::move( task ) );
                            events.emplace( num, Events::CheckTask );
                        }
                        else
                            events.emplace( num, Events::GetTask );
                    } );

            /// 6. Проверка задачи на завершенность
            func.emplace(
                Events::CheckTask,
                [ & ]( ftcl_context< _TypeTask > &context, std::size_t num )
                {
                    if( context.thread_task.taskIsReady( ) )
                    {
                        auto task = context.thread_task.getTask( );
                        in::Stream ss;
                        ss << task;
                        context.serialized_task = ss.str( );
                        events.emplace( num, Events::SendOutTask );
                        context.request_sendouttask.reset( );
                    }
                    else
                        events.emplace( num, Events::CheckTask );
                } );

            /// 7. Отправка решенной задачи на мастера
            func.emplace(
                Events::SendOutTask,
                [ & ]( ftcl_context< _TypeTask > &context, std::size_t num )
                {
                    if( !context.request_sendouttask.sendTimedMessage( num, TypeMessage::MessageTaskResponse, context.serialized_task ) )
                        events.emplace( num, Events::SendOutTask );
                    else
                        events.emplace( num, Events::GetReqTask );
                } );

            /*!
             * функция принятия запроса на завершение
             */
            func.emplace(
                Events::ShutDown,
                [ & ]( ftcl_context< _TypeTask > &context, std::size_t num )
                {
                    bool check;
                    NetworkModule::Status status;
                    std::tie( check, status ) = NetworkModule::Instance( ).checkMessage(
                            num,
                            TypeMessage::MessageShutdownMasterToWorker );
                    if ( check )
                    {
                        NetworkModule::Instance( ).getMessage( status );
                        if( !context.thread_task.taskIsReady( ) )
                        {
                            events.emplace( num, Events::ShutDownWaitTask );
                        }
                        else
                        {
                            events.emplace( num, Events::ShutDownReq );
                        }
                    }
                    else
                        events.emplace( num, Events::ShutDown );
                } );

            func.emplace(
                Events::ShutDownWaitTask,
                [ & ]( ftcl_context< _TypeTask > &context, std::size_t num )
                {
                    if( ( !context.thread_task.taskIsReady( ) ) || ( !context.request_sendouttask.checkSended( ) ) )
                    {
                        std::this_thread::sleep_for( std::chrono::milliseconds{ 1 } );
                        events.emplace( num, Events::ShutDownWaitTask );
                    }
                    else
                        events.emplace( num, Events::ShutDownReq );
                } );

            func.emplace(
                Events::ShutDownReq,
                [ & ]( ftcl_context< _TypeTask > &context, std::size_t num )
                {
                    if( !context.request_shutdown.sendTimedMessage( num, TypeMessage::MessageShutdownWorkerToMaster ) )
                        events.emplace( num, Events::ShutDownReq );
                    else
                    {
                        std::queue< std::tuple< std::size_t, Events > > q;
                        std::swap( q, events );
                    }
                } );

            func.emplace(
                Events::ShutDownForce,
                [ & ]( ftcl_context< _TypeTask > &context, std::size_t num )
                {
                    bool check;
                    NetworkModule::Status status;
                    std::tie( check, status ) = NetworkModule::Instance( ).checkMessage(
                            0,
                            TypeMessage::MessageShutDownForce );
                    if ( check )
                    {
                        NetworkModule::Instance( ).getMessage( status );
                        std::queue< std::tuple< std::size_t, Events > > q;
                        std::swap( q, events );
                    }
                    else
                        events.emplace( num, Events::ShutDownForce );
                } );

        }

        void run( ftcl_context< _TypeTask > &__context )
        {
            using namespace console;
            while ( !events.empty( ) )
            {
                std::uint64_t numWorker;
                Events event;
                std::tie( numWorker, event ) = events.front( );
                events.pop( );
                func[ event ]( __context, numWorker );
            }
            std::this_thread::sleep_for( std::chrono::seconds{ 1 } );
        }

        ~ShedulerWorker( ) = default;
    };
}

#endif
