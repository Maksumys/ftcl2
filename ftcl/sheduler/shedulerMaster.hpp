#ifndef FTCL_SHEDULER_MASTER_HPP_INCLUDEDD
#define FTCL_SHEDULER_MASTER_HPP_INCLUDEDD

#include "ftcl/network.hpp"
#include "ftcl/sheduler/sheduler.hpp"
#include "ftcl/sheduler/status.hpp"
#include "ftcl/exception.hpp"
#include "ftcl/timer.hpp"
#include "ftcl/checkpoint.hpp"
#include <queue>

namespace ftcl
{
    template< typename _TypeTask, bool with2masters = false >
    class ShedulerMaster : public Sheduler< _TypeTask >
    {
    protected:
        using NumWorker = std::size_t;

        std::shared_ptr< master_context< _TypeTask > >              context;
        std::map< Events, std::function< void( std::size_t ) > >    func;
        std::thread                                                 *thread{ nullptr };
        CheckPoint< _TypeTask >                                     checkpoint;

        using Sheduler<_TypeTask>::events;
    public:

        std::shared_ptr< master_context< _TypeTask > > getContext( )
        {
            return context;
        }

        virtual ~ShedulerMaster( )
        {
            if( thread != nullptr )
                thread -> join( );
        }

        void enableCheckPoint( const std::string &__file_name )
        {
            context->isEnableCheckPoint = true;
            checkpoint.setName( __file_name );
            events.emplace( 0, Events::InitializeCheckPoint );
        }

        explicit ShedulerMaster( )
        {
            using namespace console;
            context = std::make_shared< master_context< _TypeTask > >(  );
            context->statuses.init( ( with2masters ) ?
                                    static_cast< std::uint64_t >( NetworkModule::Instance( ).getSize( ) - 2 ) :
                                    static_cast< std::uint64_t >( NetworkModule::Instance( ).getSize( ) - 1 ),
                                    with2masters );
            context->count_task = 0;
            context->count_out_task = 0;
            context->count_sended_task = 0;

            Log( ) << "Create sheduler Master";

            /// принятие сообщений об инициализации воркеров
            for( std::size_t i = 1; i < NetworkModule::Instance( ).getSize( ); i++ )
                events.emplace( i, Events::GetInitializeWorker );

            events.emplace( 0, Events::CheckStatus );

            func.emplace(
                Events::InitializeCheckPoint,
                [ & ]( std::uint64_t numWorker )
                {
                    if( checkpoint.findCheckPoint( ) )
                    {
                        Log( ) << console::extensions::Level::Info << "Checkpoint found!";
                        checkpoint.load( getContext( ) );
                    }
                    else
                    {
                        Log( ) << console::extensions::Level::Warning << "Checkpoint not found!";
                    }
                } );

            /// 1. Ожидание инициализации воркеров
            func.emplace(
                Events::GetInitializeWorker,
                [ & ]( std::uint64_t numWorker )
                {
                    try
                    {
                        bool check;
                        NetworkModule::Status status;
                        std::tie( check, status ) = NetworkModule::Instance( ).checkMessage( numWorker, TypeMessage::WorkerInitialize );

                        if( check )
                        {
                            NetworkModule::Instance( ).getMessage( status );


                            context->statuses.getWorkerFromNumNode( numWorker ).state = State::initialize;
                            events.emplace( numWorker, Events::WorkersNameReq );
                            context->statuses.printStatusWorkers( );
                        }
                        else
                            events.emplace( numWorker, Events::GetInitializeWorker );
                    }
                    catch( std::exception &ex )
                    {
                        ex.what( );
                    }
                } );


            /// 2. Отправка запроса на имя воркера
            func.emplace(
                Events::WorkersNameReq,
                [ & ]( NumWorker numWorker )
                {
                    try
                    {
                        if( context->statuses.getWorkerFromNumNode( numWorker ).state == State::initialize )
                        {
                            /// отправляем запрос на имя воркера
                            if( context->statuses.getWorkerFromNumNode( numWorker ).worker_name_request.sendTimedMessage(
                                    numWorker,
                                    TypeMessage::MessageWorkerName ) )
                            {
                                events.emplace( numWorker, Events::GetWorkersName );
                                context->statuses.getWorkerFromNumNode( numWorker ).state = State::waitingName;
                                context->statuses.printStatusWorkers( );
                            }
                            else
                                events.emplace( numWorker, Events::WorkersNameReq );
                        }
                        else
                        {
                            context->statuses.getWorkerFromNumNode( numWorker ).state = State::failing;
                            context->statuses.printStatusWorkers( );
                            throw exception::Illegal_state_worker( __FILE__, __LINE__ );
                        }
                    }
                    catch( Exception &ex )
                    {
                        ex.what( );
                    }
                } );

            /// 3. Ожидание имени воркера
            func.emplace(
                        Events::GetWorkersName,
                        [ & ]( NumWorker numWorker )
                        {
                            try
                            {
                                if( context->statuses.getWorkerFromNumNode( numWorker ).state == State::waitingName )
                                {
                                    bool check;
                                    NetworkModule::Status status;
                                    std::tie( check, status ) = NetworkModule::Instance( ).checkMessage( numWorker, TypeMessage::requestWorkersName );
                                    if( check )
                                    {
                                        context->statuses.getWorkerFromNumNode( numWorker ).name = NetworkModule::Instance( ).getMessage( status );
                                        events.emplace( numWorker, Events::GetReqTask );
                                        context->statuses.getWorkerFromNumNode( numWorker ).state = State::waitingReqTask;
                                        Log( ) << "Master recv worker name( "
                                               << context->statuses.getWorkerFromNumNode( numWorker ).name
                                               << " )! NumWorker: "
                                               << numWorker;
                                        context->statuses.printStatusWorkers( );
                                    }
                                    else
                                        events.emplace( numWorker, Events::GetWorkersName );
                                }
                                else
                                {
                                    context->statuses.getWorkerFromNumNode( numWorker ).state = State::failing;
                                    context->statuses.printStatusWorkers( );
                                    throw exception::Illegal_state_worker( __FILE__, __LINE__ );
                                }
                            }
                            catch( Exception &ex )
                            {
                                ex.what( );
                            }
                        } );

            /// 4. Ожидание запроса на задачу
            func.emplace(
                    Events::GetReqTask,
                    [ & ]( NumWorker numWorker )
                    {
                        if( context->statuses.getWorkerFromNumNode( numWorker ).state == State::waitingReqTask )
                        {
                            bool check;
                            NetworkModule::Status status;
                            std::tie( check, status ) = NetworkModule::Instance( ).checkMessage(
                                    static_cast< const NetworkModule::Number >( numWorker ), TypeMessage::MessageReqTask );

                            if( check )
                            {
                                NetworkModule::Instance( ).getMessage( status );
                                context->statuses.getWorkerFromNumNode( numWorker ).state = State::waitingTaskSer;
                                events.emplace( numWorker, Events::SerializeTask );
                                context->statuses.printStatusWorkers( );
                            }
                            else
                                events.emplace( numWorker, Events::GetReqTask );
                        }
                        else
                        {
                            context->statuses.getWorkerFromNumNode( numWorker ).state = State::failing;
                            context->statuses.printStatusWorkers( );
                            throw exception::Illegal_state_worker( __FILE__, __LINE__ );
                        }
                    }
                );

            /// 5. Взятие свободной задачи и сериализация
            func.emplace(
                        Events::SerializeTask,
                        [ & ]( NumWorker numWorker )
                        {
                            try
                            {
                                if( context->statuses.getWorkerFromNumNode( numWorker ).state == State::waitingTaskSer )
                                {
                                    if( context->count_out_task != context->count_task )
                                    {
                                        if( !context->paramTasks.empty( ) )
                                        {
                                            in::Stream stream;
                                            stream << std::get< 0 >( context->paramTasks.front( ) );
                                            stream << std::get< 1 >( context->paramTasks.front( ) );
                                            context->statuses.getWorkerFromNumNode( numWorker ).serialized_task = stream.str( );
                                            context->paramTasks.pop( );
                                            events.emplace( numWorker, Events::SendTask );
                                            context->statuses.getWorkerFromNumNode( numWorker ).state = State::waitingTask;
                                            context->statuses.getWorkerFromNumNode( numWorker ).task_request.reset( );
                                        }
                                        else
                                        {
                                            std::this_thread::sleep_for( std::chrono::milliseconds{ 1 } );
                                            events.emplace( numWorker, Events::SerializeTask );
                                        }
                                    }
                                    /// иначе ничего не делаем
                                }
                                else
                                {
                                    context->statuses.getWorkerFromNumNode( numWorker ).state = State::failing;
                                    context->statuses.printStatusWorkers( );
                                    throw exception::Illegal_state_worker( __FILE__, __LINE__ );
                                }
                            }
                            catch( std::exception &ex )
                            {
                                std::cout << ex.what( ) << std::endl;
                            }
                        } );

            /// 6. Отправка задачи
            func.emplace(
                        Events::SendTask,
                        [ & ]( NumWorker numWorker )
                        {
                            if( context->statuses.getWorkerFromNumNode( numWorker ).state == State::waitingTask )
                            {
                                try
                                {
                                    /// отправляем задачу
                                    if( context->statuses.getWorkerFromNumNode( numWorker ).task_request.sendTimedMessage(
                                            numWorker,
                                            TypeMessage::MessageTask,
                                            context->statuses.getWorkerFromNumNode( numWorker ).serialized_task ) )
                                    {
                                        events.emplace( numWorker, Events::SendOutTask );
                                        context->statuses.getWorkerFromNumNode( numWorker ).state = State::working;
                                        context->statuses.printStatusWorkers( );
                                        context->count_sended_task++;
                                    }
                                    else
                                        events.emplace( numWorker, Events::SendTask );
                                }
                                catch( ... )
                                {
                                    /// помещаем задачу обратно
                                    _TypeTask task;
                                    std::uint64_t num_task = 0;
                                    out::Stream stream( context->statuses.getWorkerFromNumNode( numWorker ).serialized_task );
                                    stream >> task;
                                    stream >> num_task;
                                    context->paramTasks.emplace( task, num_task );

                                    context->statuses.getWorkerFromNumNode( numWorker ).state = State::failing;
                                    context->statuses.printStatusWorkers( );
                                }
                            }
                            else
                            {
                                context->statuses.getWorkerFromNumNode( numWorker ).state = State::failing;
                                context->statuses.printStatusWorkers( );

                                Log( ) << "Помещаем задачу обратно";

                                /// помещаем задачу обратно
                                _TypeTask task;
                                std::uint64_t num_task = 0;
                                out::Stream stream( context->statuses.getWorkerFromNumNode( numWorker ).serialized_task );
                                stream >> task;
                                stream >> num_task;
                                context->paramTasks.emplace( task, num_task );

                                throw exception::Illegal_state_worker( __FILE__, __LINE__ );
                            }
                        } );

            func.emplace(
                        Events::SendOutTask,
                        [ & ]( NumWorker numWorker )
                        {
                            try
                            {
                                if( context->statuses.getWorkerFromNumNode( numWorker ).state == State::working )
                                {
                                    bool check;
                                    NetworkModule::Status status;
                                    std::tie( check, status ) = NetworkModule::Instance( ).checkMessage( numWorker, TypeMessage::MessageTaskResponse );

                                    if( check )
                                    {
                                        auto serializedOutTask = NetworkModule::Instance( ).getMessage( status );

                                        _TypeTask task;
                                        std::uint64_t num_task = 0;
                                        out::Stream stream( serializedOutTask );
                                        stream >> task;
                                        stream >> num_task;
                                        context->paramOutTasks.emplace( task, num_task );


                                        /// сохранение в контрольную точку
                                        if( context->isEnableCheckPoint )
                                        {
                                            checkpoint.save( task, num_task );
                                        }


                                        context->count_out_task++;
                                        context->statuses.getWorkerFromNumNode( numWorker ).serialized_task = " ";
                                        context->statuses.getWorkerFromNumNode( numWorker ).state = State::waitingReqTask;
                                        events.emplace( numWorker, Events::GetReqTask );
                                        context->statuses.printStatusWorkers( );
                                    }
                                    else
                                        events.emplace( numWorker, Events::SendOutTask );
                                }
                                else
                                {
                                    Log( ) << "Неправильное состояние";

                                    context->statuses.getWorkerFromNumNode( numWorker ).state = State::failing;
                                    context->statuses.printStatusWorkers( );

                                    /// помещаем задачу обратно
                                    _TypeTask task;
                                    std::uint64_t num_task = 0;
                                    out::Stream stream( context->statuses.getWorkerFromNumNode( numWorker ).serialized_task );
                                    stream >> task;
                                    stream >> num_task;
                                    context->paramTasks.emplace( task, num_task );
                                    throw exception::Illegal_state_worker( __FILE__, __LINE__ );
                                }
                            }
                            catch( std::exception &ex )
                            {
                                std::cout << ex.what( ) << std::endl;
                            }
                        } );

            func.emplace( Events::CheckStatus,
                        [ & ]( NumWorker numWorker )
                        {
                            if( ( context->count_out_task < context->count_task ) && ( context->cout_failed_workers >= context->statuses.workers.size( ) ) )
                            {
                                Log( ) << console::extensions::Level::Error << "Все воркеры умерли";
                                throw std::exception( );
                            }

                            if( ( context->count_out_task == context->count_task ) && ( context->isShutDown ) )
                            {
                                for( std::uint64_t i = 0; i < context->statuses.workers.size( ); i++ )
                                {
                                    events.emplace( i + 1, Events::ShutDown );
                                }
                            }
                            else
                                events.emplace( 0, Events::CheckStatus );
                        });

            func.emplace(
                Events::ShutDown,
                [ & ]( NumWorker numWorker )
                {
                    try
                    {
                        /// отправляем завершение
                        if( context->statuses.getWorkerFromNumNode( numWorker ).shut_down_request.sendTimedMessage(
                                numWorker,
                                TypeMessage::MessageShutdownMasterToWorker ) )
                        {
                            events.emplace( numWorker, Events::ShutDownReq );
                            context->statuses.getWorkerFromNumNode( numWorker ).state = State::shutdowning;
                            context->statuses.printStatusWorkers( );
                        }
                        else
                            events.emplace( numWorker, Events::ShutDown );
                    }
                    catch( std::exception &ex )
                    {
                        std::cout << ex.what( ) << std::endl;
                    }
                } );

            func.emplace(
                        Events::ShutDownReq,
                        [ & ]( NumWorker numWorker )
                        {
                            bool check;
                            NetworkModule::Status status;
                            std::tie( check, status ) = NetworkModule::Instance( ).checkMessage(
                                    numWorker,
                                    TypeMessage::MessageShutdownWorkerToMaster );
                            if ( !check )
                            {
                                events.emplace( numWorker, Events::ShutDownReq );
                            }
                            else
                            {
                                NetworkModule::Instance( ).getMessage( status );
                                context->statuses.getWorkerFromNumNode( numWorker ).state = State::readyToShutDown;
                                context->count_shutdown++;
                                context->statuses.printStatusWorkers( );
                            }
                        } );
        }

        void setTask( _TypeTask &&task )
        {
            context->paramTasks.push( std::make_tuple( task, context->count_task ) );
            context->count_task++;
        }


        bool isOutTask( )
        {
            return !context->paramOutTasks.empty( );
        }

        _TypeTask getTask( )
        {
            _TypeTask task = std::get< 0 >( context->paramOutTasks.front( ) );
            context->paramOutTasks.pop( );
            return task;
        }

        void shutDown( )
        {
            while( ( context->count_out_task != context->count_task ) && ( context->cout_failed_workers < context->statuses.workers.size( ) ) )
            {
                std::this_thread::sleep_for( std::chrono::milliseconds{ 2 } );
            }
            context->isShutDown = true;
            std::this_thread::sleep_for( std::chrono::seconds{ 5 } );
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
                    else
                    {
                        _TypeTask task;
                        std::uint64_t num_task;
                        out::Stream stream( context->statuses.getWorkerFromNumNode( numWorker ).serialized_task );
                        stream >> task;
                        stream >> num_task;
                        context->paramTasks.emplace( task, num_task );
                        Log( ) << "Delete events numWorker = " << numWorker << " event = " << ( int ) event;
                    }
                    events.pop( );
                }

                for( std::size_t i = 0; i < failingProc.size( ); i++ )
                {
                    events.emplace( NetworkModule::Instance( ).getSize( ) - i - 1, Events::GetInitializeWorker );
                    Log( ) << "Rank: " << NetworkModule::Instance( ).getSize( ) - i - 1 << " Events::GetInitializeWorker";
                }

                for( std::size_t i = proc; i < context->statuses.workers.size( ); i++ )
                    context->statuses.workers[ i - 1 ] = context->statuses.workers[ i ];
            }

            for( std::size_t i = 0; i < failingProc.size( ); i++ )
            {
                context->statuses.workers[ context->statuses.workers.size( ) - 1 - i ] = Worker{ };
            }
            context->statuses.printStatusWorkers( );
        }

        void run( )
        {
            thread = new std::thread( [ & ]( ){
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
                    std::this_thread::sleep_for( std::chrono::milliseconds{ 10 } );
                }

                context->statuses.printStatusWorkers( );

                if( context->cout_failed_workers > context->statuses.workers.size( ) )
                {
                    Log( ) << "ABORT!";
                    std::this_thread::sleep_for( std::chrono::seconds{ 2 } );
                    NetworkModule::Instance( ).abort( );
                }
            } );
        }
    };
}

#endif
