#include "ftcl/sheduler/status.hpp"
#include "ftcl/exception.hpp"
#include "ftcl/console/log.hpp"

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

    void StatusWorker::getWorkersName( const std::size_t __numWorkers )
    {
        if( __numWorkers >= NetworkModule::Instance( ).getSize( ) )
            throw exception::Illegal_rank( __FILE__, __LINE__ );
        if( ( statuses[ __numWorkers - 1 ].state != State::initialize ) || ( statuses[ __numWorkers - 1 ].state != State::waitingName ) )
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
                statuses[ __numWorkers - 1 ].name = NetworkModule::Instance( ).getMessage( status );
                statuses[ __numWorkers - 1 ].state = State::waitingTask;
            }
            if( statuses[ __numWorkers - 1 ].timeCurrentState.end( ) >= 3 )
            {
                NetworkModule::Instance( ).cancel( statuses[ __numWorkers - 1 ].workerNameRequest );
                statuses[ __numWorkers - 1 ].state = State::failing;
            }
        }
    }

    void StatusWorker::printStatusWorkers( )
    {
        using console::Log;
        Log( ) << "Status workers:\n\n";
        int i = 0;
        for( const auto &elem : statuses )
        {
            switch( elem.state )
            {
                case State::idle:
                    Log( ) << "NumWorker " << i << "state idle";
                    break;
                case State::initialize:
                    Log( ) << "NumWorker " << i << "state initialize";
                    break;
                case State::working:
                    Log( ) << "NumWorker " << i << "state working";
                    break;
                case State::failing:
                    Log( ) << "NumWorker " << i << "state failing";
                    break;
                case State::shutdowning:
                    Log( ) << "NumWorker " << i << "state shutdowning";
                    break;
            }
            i++;
        }
        Log( ) << "\n\n";
    }
}
