#include "ftcl/sheduler/status.hpp"
#include "ftcl/exception.hpp"
#include "ftcl/console/log.hpp"

#include <string>

namespace ftcl
{
    StatusWorker::StatusWorker( const std::uint64_t &__countWorkers )
    {
        statuses.resize( __countWorkers );

        for( auto &stat : statuses )
            stat.state = State::idle;
    }
    bool StatusWorker::isAllInitialize( )
    {
        return false;
    }

    bool StatusWorker::recvInitialize( const std::size_t __numWorkers )
    {
        if( __numWorkers >= NetworkModule::Instance( ).getSize( ) )
            throw exception::Illegal_rank( __FILE__, __LINE__ );
        if( statuses[ __numWorkers - 1 ].state != State::idle )
            throw exception::Illegal_state_worker( __FILE__, __LINE__ );
        auto[ check, status ] = NetworkModule::Instance( ).checkMessage( __numWorkers, TypeMessage::WorkerInitialize );
        if( check )
        {
            NetworkModule::Instance( ).getMessage( status );
            statuses[ __numWorkers - 1 ].state = State::initialize;
            console::Log( ) << "initialized " << __numWorkers;
            countInitializedWorkers++;
        }
        return check;
    }

    bool StatusWorker::getWorkersName( const std::size_t __numWorkers )
    {
        if( __numWorkers >= NetworkModule::Instance( ).getSize( ) )
            throw exception::Illegal_rank( __FILE__, __LINE__ );
        if( ( statuses[ __numWorkers - 1 ].state != State::initialize ) && ( statuses[ __numWorkers - 1 ].state != State::waitingName ) )
            throw exception::Illegal_state_worker( __FILE__, __LINE__ );

        if( statuses[ __numWorkers - 1 ].state == State::initialize )
        {
            statuses[ __numWorkers - 1 ].workerNameRequest = NetworkModule::Instance( ).send( __numWorkers, TypeMessage::MessageWorkerName );
            statuses[ __numWorkers - 1 ].timeCurrentState.start( );
            statuses[ __numWorkers - 1 ].state = State::waitingName;
        }
        if( statuses[ __numWorkers - 1 ].state == State::waitingName )
        {
            auto[ check, status ] = NetworkModule::Instance( ).checkMessage( __numWorkers, TypeMessage::MessageWorkerName );
            if( check )
            {
                auto buf = NetworkModule::Instance( ).getMessage( status );
                for( std::size_t i = 0; i < buf.size( ); i++ )
                {
                    statuses[ __numWorkers - 1 ].name += buf[ i ];
                }

                ///////// TODO! изменить потом на waitingTask

                statuses[ __numWorkers - 1 ].state = State::readyToShutDown;
            }
            if( statuses[ __numWorkers - 1 ].timeCurrentState.end( ) >= 3 )
            {
                NetworkModule::Instance( ).cancel( statuses[ __numWorkers - 1 ].workerNameRequest );
                statuses[ __numWorkers - 1 ].state = State::failing;
            }
            return check;
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
                    str += "NumWorker " + std::to_string( i ) + "  state idle\n";
                    break;
                case State::initialize:
                    str += "NumWorker " + std::to_string( i ) + "  state initialize\n";
                    break;
                case State::working:
                    str += "NumWorker " + std::to_string( i ) + "  state working\n";
                    break;
                case State::failing:
                    str += "NumWorker " + std::to_string( i ) + "  state failing\n";
                    break;
                case State::shutdowning:
                    str += "NumWorker " + std::to_string( i ) + "  state shutdowning\n";
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
            statuses[ __numWorkers - 1 ].timeCurrentState.start( );
            statuses[ __numWorkers - 1 ].sendedShutDown = true;
        }
        else
        {
            auto[ check, status ] = NetworkModule::Instance( ).checkMessage( __numWorkers, TypeMessage::MessageShutdownWorkerToMaster );
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
}
