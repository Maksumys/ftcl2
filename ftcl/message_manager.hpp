#ifndef FTCL_MESSAGE_MANAGER_HPP
#define FTCL_MESSAGE_MANAGER_HPP

#include <map>
#include <thread>
#include <mutex>
#include <queue>
#include <optional>

#include "ftcl/network.hpp"
#include "ftcl/message.hpp"
#include "ftcl/queue.hpp"

namespace ftcl
{
    class messageManager
    {
    protected:
        std::map< std::size_t, queue< std::string > > inMessages;              ///< входящие сообщения
        queue< std::tuple< 
                           std::size_t,                                        ///< номер узла
                           NetworkModule::Request,                             ///< ссылка на пересылку
                           bool,                                               ///< неудачная пересылка
                           std::string                                         ///< сериализованные данные
                          >
             >                                         outMessages{ 1000 };    ///< исходящие сообщения
        std::thread                                    *thread{ nullptr };     ///< управляющий поток
        std::mutex                                     mutex;                  ///< синхронизация доступа
        bool                                           isStop{ true };         ///< признак остановки управляющего потока

        messageManager( const messageManager& ) = delete;
        messageManager( messageManager&& ) = delete;
        messageManager operator=( const messageManager& ) = delete;
        messageManager operator=( messageManager&& ) = delete;
        
        messageManager( )
        {
            registerMessage< message0 >( );
            registerMessage< message1 >( );
            registerMessage< message2 >( );
            isStop = false;
            if( thread == nullptr )
                thread = new std::thread( &run, this );
        }
    public:
        messageManager& Instance( )
        {
            static messageManager maneger;
            return maneger;
        }

        template< class TypeMessage >
        void registerMessage( )
        {
            std::lock_guard lock( mutex );
            inMessages.emplace( typeid( TypeMessage ).hash_code( ), queue{ } );
        }
        
        template < class TypeMessage >
        std::optional< TypeMessage >
        getMessage( )
        {
            std::lock_guard lock( mutex );
            if( !inMessages[ typeid( TypeMessage ).hash_code( ) ].empty( ) )
            {
                auto buf = inMessages[ typeid( TypeMessage ).hash_code( ) ].front( );
                inMessages[ typeid( TypeMessage ).hash_code( ) ].pop( );
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
        
        template< class TypeMessage >
        void setMessage( TypeMessage &&message )
        {
            std::stringstream stream;
            std::size_t to_rank = message.to_rank;
            message.from_rank = NetworkModule::Instance( ).getRank( );
            stream << typeid( TypeMessage ).hash_code( );
            stream << message;
            outMessages.push( std::make_tuple( to_rank, NetworkModule::Request{ }, bool{ false }, message ) );
        }

        void run( )
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
                    inMessages[ hash ].push( std::move( stream ) );
                }

                if( !outMessages.empty( ) )
                {
                    auto [ rank, req, isFail, serializedMess ] = outMessage.front( );
                    NetworkModule::Instance( ).send( outMessage.front( ) );
                    outMessage.pop( );
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
