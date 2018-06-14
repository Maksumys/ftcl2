#ifndef FTCL_SHEDULER_HPP_INCLUDED
#define FTCL_SHEDULER_HPP_INCLUDED

#include "ftcl/console/log.hpp"
#include "ftcl/timer.hpp"
#include "ftcl/network.hpp"

#include <queue>
#include <functional>
#include <map>

namespace ftcl
{
    enum class Events
    {
        GetInitializeWorker,
        WorkersNameReq,
        CheckWorkersName,
        GetWorkersName,
        GetReqTask,                     ///< запрос на новую задачу         ( мастер <- воркер )
        GetTask,                        ///< получение задачи               ( мастер -> воркер )
        CheckTask,                      ///< проверка задачи на решенность  ( воркер <-> воркер )
        SerializeTask,
        SendTask,                       ///< отправка задачи                ( мастер -> воркер )
        SendOutTask,                    ///< отправка решенной задачи       ( мастер <- воркер )
        ShutDown,
        ShutDownWaitTask,               ///< ожидание завершения задачи     ( воркер <-> воркер )
        ShutDownReq,
        ShutDownForce,
        CheckStatus,
        InitializeCheckPoint,           ///< инициализация контрольной точки( в том числе считывание текущей )
    };

    template< typename _Task, typename = std::enable_if_t< std::is_default_constructible< _Task >::value > >
    class thread
    {
    protected:
        std::thread *thr;
        std::condition_variable cond_var;
        std::mutex mutex;
        bool notified{ false };
        bool is_run{ true };
        _Task task;
        std::uint64_t num_task;
    public:
        thread( )
        {
            thr = new std::thread( &thread::run, this );
        }

        void setTask( const _Task &__task )
        {
            std::unique_lock<std::mutex> lock( mutex );
            task = __task;
            notified = true;
            cond_var.notify_all( );
        }

        void setTask( _Task &&__task, const std::uint64_t __num_task )
        {
            std::unique_lock<std::mutex> lock( mutex );
            task = std::move( __task );
            num_task = __num_task;
            notified = true;
            cond_var.notify_all( );
        }

        bool taskIsReady( )
        {
            return !notified;
        }

        _Task getTask( )
        {
            return task;
        }

        std::uint64_t getNumTask( )
        {
            return num_task;
        }

        void run( )
        {
            while( is_run )
            {
                std::unique_lock< std::mutex > lock( mutex );
                while( !notified )
                {
                    cond_var.wait( lock );
                    if( !is_run )
                        return;
                }
                task.run( );
                notified = false;
            }
        }

        ~thread( )
        {
            is_run = false;
            cond_var.notify_all( );
            if( thr != nullptr )
            {
                thr -> join( );
                delete thr;
            }
        }
    };

    enum class State
    {
        idle = 0,
        initialize,
        waitingName,
        waitingReqTask,
        waitingTask,
        waitingTaskSer,
        working,
        failing,
        readyToShutDown,
        shutdowning,
    };

    class SendedRequest
    {
    public:
        NetworkModule::Request request;
        bool sended{ false };
        Timer timer;

        bool checkSended( )
        {
            return sended;
        }

        void reset( )
        {
            sended = false;
        }

        bool sendTimedMessage( const std::size_t num, const TypeMessage __type, const std::string &str = " " )
        {
            /// Если была ошибка отменяем запрос
            /*if( NetworkModule::Instance( ).getError( ) )
            {
                console::Log( ) << console::extensions::Level::Error << "Отмена запроса Type: " << ( int )__type;
                sended = false;
                NetworkModule::Instance( ).cancel( request );
                NetworkModule::Instance( ).resetError( );
            }*/

            if( !sended )
            {
                timer.start( );
                request = NetworkModule::Instance( ).send( str, num, __type );
                sended = true;
                return false;
            }
            else
            {
                /// Проверяем что отправляли сообщение
                NetworkModule::Status status;
                auto test = NetworkModule::Instance( ).test( request, status );

                /// Если сообщение отправлено
                if( test )
                {
                    if( status.MPI_SOURCE != -1 )
                    {
                        return true;
                    }
                    else
                    {
                        sended = false;
                        console::Log( ) << "Worker cancel2! Type:" << ( int )__type;
                        NetworkModule::Instance( ).cancel( request );
                    }
                }

                /// Если вышло время отменяем запрос и все!
                if( timer.end( ) < 40000 )
                    return false;
                else
                {
                    console::Log( ) << "Worker cancel3! Type:" << ( int )__type;
                    NetworkModule::Instance( ).cancel( request );
                    sended = false;
                    throw std::exception( );
                }
            }
        }
    };

    class Worker
    {
    public:
        State state{ State::idle };
        std::string name;
        SendedRequest worker_name_request;
        SendedRequest task_request;
        SendedRequest shut_down_request;

        std::string serialized_task;
    };

    class Status
    {
    public:
        std::vector< Worker >                   workers;
        std::uint32_t                           coeff;

    public:
        Status( ) = default;
        Status( const std::uint64_t __count, bool with2masters )
        {
            workers.resize( __count );
            ( with2masters ) ? coeff = 2 : coeff = 1;
        }

        void init( const std::uint64_t __count, bool with2masters )
        {
            workers.resize( __count );
            ( with2masters ) ? coeff = 2 : coeff = 1;
        }

        void printStatusWorkers( )
        {
            using console::Log;
            std::string str;
            str += "\n\nStatus workers:\n";
            int i = 0;
            for( const auto &elem : workers )
            {
                switch( elem.state )
                {
                    case State::idle:
                        str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + elem.name + "  state idle\n";
                        break;
                    case State::waitingName:
                        str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + elem.name + "  state waitingName\n";
                        break;
                    case State::waitingTask:
                        str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + elem.name + "  state waitingTask\n";
                        break;
                    case State::initialize:
                        str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + elem.name + "  state initialize\n";
                        break;
                    case State::working:
                        str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + elem.name + "  state working\n";
                        break;
                    case State::failing:
                        str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + elem.name + "  state failing\n";
                        break;
                    case State::shutdowning:
                        str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + elem.name + "  state shutdowning\n";
                        break;
                    case State::readyToShutDown:
                        str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + elem.name + "  state readyToShutDown\n";
                        break;
                    case State::waitingReqTask:
                        str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + elem.name + "  state waitingReqTask\n";
                        break;
                    case State::waitingTaskSer:
                        str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + elem.name + "  state waitingTaskSer\n";
                        break;
                }
                i++;
            }
            Log( ) << str + "\n\n";
        }

        Worker& getWorkerFromNumNode( std::uint64_t __numNode )
        {
            return workers.at( __numNode - coeff );
        }

        Worker& getWorkerFromNum( std::uint64_t __num )
        {
            return workers.at( __num );
        }
    };

    template< typename _TypeTask >
    class ftcl_context
    {
    public:
        using NumWorker = std::size_t;

        SendedRequest request_init;
        SendedRequest request_name;
        SendedRequest request_shutdown;
        SendedRequest request_gettask;
        SendedRequest request_sendouttask;

        thread< _TypeTask > thread_task;
        std::string serialized_task;
    };

    template< class _TypeTask >
    class master_context
    {
    public:
        Status                              statuses;                       ///< статусы воркеров

        std::queue< std::tuple< _TypeTask, std::uint64_t > > paramTasks;    ///< очередь задач

        std::queue< std::tuple< _TypeTask, std::uint64_t > > paramOutTasks; ///< очередь решенных задач

        std::size_t                         count_sended_task;              ///< количество отправленных задач
        std::size_t                         count_out_task;                 ///< количество решенных задач
        std::size_t                         count_task;                     ///< количество задач
        std::size_t                         cout_failed_workers;            ///< количество умерших воркеров
        std::size_t                         count_shutdown;                 ///< количество завершенных мастеров
        bool                                isShutDown{ false };            ///< пользователь инициировал завершение программы
        bool                                isEnableCheckPoint{ false };    ///< создание контрольных точек
    };

    template< typename _TypeTask >
    class Sheduler
    {
    protected:
        using NumWorker = std::size_t;

        std::queue< std::tuple< std::size_t, Events > > events;
    public:
        Sheduler( ) = default;
        virtual ~Sheduler( ) = default;

        Sheduler( Sheduler& ) = delete;
        Sheduler& operator=( const Sheduler& ) = delete;
    };
}

#endif // FTCL_SHEDULER_HPP_INCLUDED
