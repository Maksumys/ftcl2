///TODO:
#define MPI_INCLUDED

#ifdef MPI_INCLUDED
#ifndef _FTCL_SHEDULER_SHEDULER_HPP_INCLUDED
#define _FTCL_SHEDULER_SHEDULER_HPP_INCLUDED

#include "ftcl/multithread/queue.hpp"
#include "ftcl/network.hpp"
#include "ftcl/console/log.hpp"
#include <thread>


namespace ftcl { namespace sheduler {

    enum class TypeAction
    {
        LogOfWorkers
    };

    class Sheduler
    {
    protected:
        Sheduler( )
        {
            thread = new std::thread(  );
        }

        void run( )
        {
            while( ( !exit ) && ( !queue.empty( ) ) )
            {
                if( !queue.empty( ) )
                {
                    switch( queue.back( ) )
                    {
                        case TypeAction::LogOfWorkers:
                        {
                            auto check = NetworkModule::Instance( ).checkMessage( -1, TypeMessage::MessageLog );
                            if( std::get< 0 >( check ) )
                            {
                                auto message = NetworkModule::Instance( ).getMessage( std::get< 1 >( check ) );
                                console::Log( ) <<  << message;
                            }
                            break;
                        }
                    }
                    queue.push( queue.back( ) );
                    queue.pop( );
                }
            }
        }

        Sheduler( Sheduler& ) = delete;
        Sheduler& operator=( const Sheduler& ) = delete;

        multithread::queue< TypeAction > queue{ 1000 };
        std::thread *thread;
        bool exit{ false };
    public:
       static Sheduler& Instance( )
       {
           static Sheduler sheduler;
           return sheduler;
       }
    };

    Sheduler& getSheduler( )
    {
        return Sheduler::Instance( );
    }

} }

#endif // _FTCL_SHEDULER_SHEDULER_HPP_INCLUDED
#endif
