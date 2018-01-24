//
// Created by Максим Кузин on 23.01.2018.
//

#ifndef FTCL_MESSAGE_MANAGER_HPP
#define FTCL_MESSAGE_MANAGER_HPP

#include <map>
#include <thread>
#include <mutex>
#include <queue>
#include <optional>

#include "ftcl/network.hpp"
#include "ftcl/message.hpp"

namespace ftcl
{
    class messageManager
    {
    protected:
        std::map< std::size_t, std::queue< std::string > > handler;
        std::thread *thread{ nullptr };
        std::mutex mutex;
        bool isStop{ true };
        
        messageMenager( const messageMenager& ) = delete;
        messgeMenager( messageMenager&& ) = delete;
        messageMenager operator=( const messageMenager& ) = delete;
        messageMenager operator=( messageMenager&& ) = delete;
        
        messageManager( )
        {
            registerMessage< message0 >( );
            registerMessage< message1 >( );
            registerMessage< message2 >( );
            isStop = false;
            if( thread == nullptr )
                thread = new std::thread( &runRead, this );
        }
    public:
        messageMenager& Instance( )
        {
            static messageMenager menager;
            return menager;
        }

        template< class TypeMessage >
        void registerMessage( )
        {
            std::lock_guard lock( mutex );
            handler.emplace( typeid( TypeMessage ).hash_code( ), std::queue{ } );
        }
        
        template < class TypeMessage >
        std::optional< TypeMessage >
        getMessage( )
        {
            std::lock_guard lock( mutex );
            if( !handler[ typeid( TypeMessage ).hash_code( ) ].empty( ) )
            {
                auto buf = handler[ typeid( TypeMessage ).hash_code( ) ].front( );
                handler[ typeid( TypeMessage ).hash_code( ) ].pop( );
                std::stringstream stream( buf );
                TypeMessage message;
                stream >> message;
                return message;
            }
            else
            {
                return { };
            }
        }

        void runRead( )
        {
            while( !isStop )
            {
                auto[ check, status ] = NetworkModule::Instance( ).checkMessage( );
                if( check )
                {
                    auto str = NetworkModule::Instance( ).getMessage( status );
                    std::stringstream stream( str );
                    std::size_t hash;
                    std::string string;
                    stream >> hash;
                    stream >> string;
                    std::lock_guard lock( mutex );
                    handler[ hash ].push( std::move( stream ) );
                }

                std::this_thread::sleep_for( std::chrono::milliseconds{ 1 } );
            }
        }

        void stop( )
        {
            isStop = true;
        }
        
        ~messageManager( )
        {
            isStop = true;
            if( thread != nullptr )
                thread -> join( );
        }
    };
}

#endif //FTCL_MESSAGE_MANAGER_HPP
