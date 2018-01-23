//
// Created by Максим Кузин on 23.01.2018.
//

#ifndef FTCL_MESSAGE_MANAGER_HPP
#define FTCL_MESSAGE_MANAGER_HPP

#include <map>
#include <thread>
#include <mutex>
#include <queue>

#include "ftcl/network.hpp"
#include "ftcl/message.hpp"

namespace ftcl
{
    class messageManager
    {
    public:
        std::map< std::size_t, std::queue< message* > > handler;
        std::thread *thread{ nullptr };
        std::mutex mutex;
        bool isStop{ true };


        void initialize( )
        {
            handler.emplace( typeid( message0 ).hash_code( ), std::queue{ } );
            handler.emplace( typeid( message1 ).hash_code( ), std::queue{ } );
            handler.emplace( typeid( message2 ).hash_code( ), std::queue{ } );
            isStop = false;
            if( thread == nullptr )
                thread = new std::thread( &runRead, this );
        }

        template < class TypeMessage >
        TypeMessage getMessage( )
        {
            std::lock_guard lock( mutex );
            auto buf = handler[ typeid( TypeMessage ).hash_code( ) ].front( );
            handler[ typeid( TypeMessage ).hash_code( ) ].pop( );
            return *static_cast< TypeMessage* >( buf );
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
                    handler[ hash ]( std::move( stream ) );
                }

                std::this_thread::sleep_for( std::chrono::milliseconds{ 1 } );
            }
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
