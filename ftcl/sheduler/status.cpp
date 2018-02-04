#include "ftcl/sheduler/status.hpp"

namespace ftcl
{
    StatusWorker::StatusWorker( const std::uint64_t &__countWorkers )
    {
        statuses.resize( __countWorkers );

        for( auto &stat : statuses )
            stat.state = State::idle;
    }

    bool StatusWorker::recvInitialize( const std::size_t __numWorkers )
    {
        if( __numWorkers >= NetworkModule::Instance( ).getSize( ) )
            throw exception::Illegal_rank( __FILE__, __LINE__ );
        if( statuses[ __numWorkers - 1 ].state != State::idle )
            throw exception::Illegal_state_worker( __FILE__, __LINE__ );

        bool check;
        NetworkModule::Status status;
        std::tie( check, status ) = NetworkModule::Instance( ).checkMessage( __numWorkers, TypeMessage::WorkerInitialize );

        if( check )
        {
            NetworkModule::Instance( ).getMessage( status );
            statuses[ __numWorkers - 1 ].state = State::initialize;
            console::Log( ) << "initialized " << __numWorkers;
            countInitializedWorkers++;
            statuses[ __numWorkers - 1 ].isInit = true;
        }
        return check;
    }

    bool StatusWorker::getWorkersName( const std::size_t __numWorkers )
    {
        if( TEST )
        {
            std::this_thread::sleep_for( std::chrono::seconds{ 10 } );
            TEST = false;
        }

        if( __numWorkers >= NetworkModule::Instance( ).getSize( ) )
            throw exception::Illegal_rank( __FILE__, __LINE__ );
        if( ( statuses[ __numWorkers - 1 ].state != State::initialize ) &&
            ( statuses[ __numWorkers - 1 ].state != State::waitingName ) )
            throw exception::Illegal_state_worker( __FILE__, __LINE__ );

        if( statuses[ __numWorkers - 1 ].state == State::initialize )
        {
            statuses[ __numWorkers - 1 ].workerNameRequest = NetworkModule::Instance( ).send( __numWorkers, TypeMessage::MessageWorkerName );
            if( NetworkModule::Instance( ).getError( ) )
                return false;

            statuses[ __numWorkers - 1 ].timeCurrentState.start( );
            statuses[ __numWorkers - 1 ].state = State::waitingName;
            console::Log( ) << "master send req to worker" << __numWorkers - 1;

        }
        if( statuses[ __numWorkers - 1 ].state == State::waitingName )
        {
            bool check;
            NetworkModule::Status status;
            std::tie( check, status ) = NetworkModule::Instance( ).checkMessage( __numWorkers, TypeMessage::requestWorkersName );
            if( NetworkModule::Instance( ).getError( ) )
                return false;

            if( check )
            {
                auto buf = NetworkModule::Instance( ).getMessage( status );
                if( NetworkModule::Instance( ).getError( ) )
                    return false;

                statuses[ __numWorkers - 1 ].name = buf;
                ///////// TODO! изменить потом на waitingTask
                statuses[ __numWorkers - 1 ].state = State::readyToShutDown;
            }
            if( statuses[ __numWorkers - 1 ].timeCurrentState.end( ) >= 7000 )
            {
                NetworkModule::Instance( ).cancel( statuses[ __numWorkers - 1 ].workerNameRequest );
                if( NetworkModule::Instance( ).getError( ) )
                {
                    std::cout << "get error in getWorkersName" << std::endl;
                    return false;
                }

                statuses[ __numWorkers - 1 ].state = State::failing;
            }

            /*if( NetworkModule::Instance( ).getError( ) )
            {
                auto proc = NetworkModule::Instance( ).getFailingProc( );
                for( auto &elem : proc )
                {
                    statuses[ elem - 1 ] = _StatusWorker{ };
                }
                return false;
            }*/

            return check;
        }

        if( NetworkModule::Instance( ).getError( ) )
        {
            statuses[ __numWorkers - 1 ] = _StatusWorker{ };
            return false;
        }

        return false;
    }

    void StatusWorker::printStatusWorkers( )
    {
        using console::Log;
        std::string str;
        str += "\n\nStatus workers:\n";
        int i = 0;
        for( const auto &elem : statuses )
        {
            switch( elem.state )
            {
                case State::idle:
                    str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + NetworkModule::Instance( ).getName( ) + "  state idle\n";
                    break;
                case State::waitingName:
                    str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + NetworkModule::Instance( ).getName( ) + "  state waitingName\n";
                    break;
                case State::waitingTask:
                    str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + NetworkModule::Instance( ).getName( ) + "  state waitingTask\n";
                    break;
                case State::initialize:
                    str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + NetworkModule::Instance( ).getName( ) + "  state initialize\n";
                    break;
                case State::working:
                    str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + NetworkModule::Instance( ).getName( ) + "  state working\n";
                    break;
                case State::failing:
                    str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + NetworkModule::Instance( ).getName( ) + "  state failing\n";
                    break;
                case State::shutdowning:
                    str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + NetworkModule::Instance( ).getName( ) + "  state shutdowning\n";
                    break;
                case State::readyToShutDown:
                    str += "[NumWorker " + std::to_string( i + 1 ) + "]  Name " + NetworkModule::Instance( ).getName( ) + "  state readyToShutDown\n";
                    break;
            }
            i++;
        }
        Log( ) << str + "\n\n";
    }

    std::string StatusWorker::getWorkerName( std::size_t __numWorker )
    {
        return statuses[ __numWorker - 1 ].name;
    }

    bool StatusWorker::shutDown( const std::size_t __numWorkers )
    {
        if( __numWorkers >= NetworkModule::Instance( ).getSize( ) )
            throw exception::Illegal_rank( __FILE__, __LINE__ );
        if( statuses[ __numWorkers - 1 ].state != State::readyToShutDown )
            throw exception::Illegal_state_worker( __FILE__, __LINE__ );

        if( !statuses[ __numWorkers - 1 ].sendedShutDown )
        {
            statuses[ __numWorkers - 1 ].workerShutDownRequest = NetworkModule::Instance( ).send( __numWorkers, TypeMessage::MessageShutdownMasterToWorker );
            if( NetworkModule::Instance( ).getError( ) )
                return false;

            statuses[ __numWorkers - 1 ].timeCurrentState.start( );
            statuses[ __numWorkers - 1 ].sendedShutDown = true;

            console::Log( ) << "master send shutdown";

        }
        else
        {
            bool check;
            NetworkModule::Status status;
            std::tie( check, status ) = NetworkModule::Instance( ).checkMessage( __numWorkers, TypeMessage::MessageShutdownWorkerToMaster );
            if( NetworkModule::Instance( ).getError( ) )
                return false;

            if( check )
            {
                NetworkModule::Instance( ).getMessage( status );
            }

            if( statuses[ __numWorkers - 1 ].timeCurrentState.end( ) >= 3000 )
            {
                NetworkModule::Instance( ).cancel( statuses[ __numWorkers - 1 ].workerShutDownRequest );
                statuses[ __numWorkers - 1 ].state = State::failing;
                statuses[ __numWorkers - 1 ].sendedShutDown = false;
                throw exception::Error_worker_shutDown( __FILE__, __LINE__ );
            }
            return check;
        }
        return false;
    }

    void StatusWorker::sendTask( std::size_t __numWorkers, const std::string &__str )
    {
    }

    bool StatusWorker::isInit( const std::size_t __numWorkers  )
    {
        return statuses[ __numWorkers - 1 ].isInit;
    }

    template< typename _TypeTask >
    void StatusWorker::getTask( std::size_t __numWorkers, _TypeTask &task )
    {
        if( __numWorkers >= NetworkModule::Instance( ).getSize( ) )
            throw exception::Illegal_rank( __FILE__, __LINE__ );
        if( ( statuses[ __numWorkers - 1 ].state != State::waitingReqTask ) &&
            ( statuses[ __numWorkers - 1 ].state != State::waitingTask ) )
            throw exception::Illegal_state_worker( __FILE__, __LINE__ );


        if( statuses[ __numWorkers - 1 ].state == State::waitingReqTask )
        {
            bool check;
            NetworkModule::Status status;
            std::tie( check, status ) = NetworkModule::Instance( ).checkMessage( __numWorkers, TypeMessage::MessageReqTask );
            if( check )
            {
                NetworkModule::Instance( ).getMessage( status );
                statuses[ __numWorkers - 1 ].state = State::waitingTask;
            }
        }
        else if( statuses[ __numWorkers - 1 ].state == State::waitingTask )
        {
            /// отправление задачи ( входных параметров для задачи )
        }
    }
}
