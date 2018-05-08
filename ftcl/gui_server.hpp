#ifndef FTCL_GUI_SERVER_HPP
#define FTCL_GUI_SERVER_HPP

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include "ftcl/sheduler/shedulerMaster.hpp"

namespace ftcl
{
    using boost::asio::ip::tcp;

    template< class _TypeTask >
    class session : public std::enable_shared_from_this< session< _TypeTask > >
    {
        std::shared_ptr< master_context< _TypeTask > > context;
    public:
        session( tcp::socket socket, std::shared_ptr< master_context< _TypeTask > > __context ) :
                socket_{ std::move( socket )},
                context{ __context }
        {
        }

        void
        start( )
        {
            do_read( );
        }

    private:
        std::stringstream getMessage( const std::uint64_t type, const std::string &mess )
        {
            std::stringstream ss;
            auto *ch = ( unsigned char * )&type;
            ss << ch[ 3 ];
            ss << ch[ 2 ];
            ss << ch[ 1 ];
            ss << ch[ 0 ];

            auto size = mess.size( );

            auto *ch2 = ( unsigned char * )&size;
            ss << ch2[ 3 ];
            ss << ch2[ 2 ];
            ss << ch2[ 1 ];
            ss << ch2[ 0 ];

            ss << mess;

            return ss;
        }


        void
        do_read( )
        {
            auto self( this->shared_from_this( ) );

            data_.resize( 4 );
            socket_.async_read_some(
                    boost::asio::buffer( data_ ),
                    [ &, self ]( boost::system::error_code ec, std::size_t length )
                    {
                        if ( !ec )
                        {
                            type = ( data_[ 3 ] ) | ( data_[ 2 ] << 8 ) | ( data_[ 1 ] << 16 ) | ( data_[ 0 ] << 24 );
                            read_size( );
                        }
                    } );

        }

        void read_size( )
        {
            auto self( this->shared_from_this( ) );
            data_.resize( 4 );
            socket_.async_read_some(
                    boost::asio::buffer( data_ ),
                    [ &, self ]( boost::system::error_code ec, std::size_t length )
                    {
                        if ( !ec )
                        {
                            size = ( data_[ 3 ] ) | ( data_[ 2 ] << 8 ) | ( data_[ 1 ] << 16 ) | ( data_[ 0 ] << 24 );
                            if( size != 0 )
                            {
                                read_message( );
                            }
                            else
                            {
                                console::Log( ) << "get Message: " << "type=" << type << " size=" << size;
                                send_message( );
                            }
                        }
                    } );
        }

        void send_message( )
        {
            console::Log( ) << "Send message " << type;
            if( type == 1 )
            {
                buf_send = getMessage( 10, std::to_string( context->statuses.workers.size( ) ) ).str( );
            }
            else if( type == 2 )
            {
                buf_send = getMessage( 11, std::to_string( context->statuses.coeff ) ).str( );
            }
            else if( type == 3 )
            {
                buf_send = getMessage( 12, std::to_string( context->count_sended_task ) ).str( );
            }
            else if( type == 4 )
            {
                buf_send = getMessage( 13, std::to_string( context->count_out_task ) ).str( );
            }
            else if( type == 5 )
            {
                buf_send = getMessage( 14, std::to_string( context->count_task ) ).str( );
            }
            else if( type == 6 )
            {
                buf_send = getMessage( 15, std::to_string( context->cout_failed_workers ) ).str( );
            }
            else if( type == 7 )
            {
                buf_send = getMessage( 16, std::to_string( context->count_shutdown ) ).str( );
            }
            else if( type == 8 )
            {
                buf_send = getMessage( 17, std::to_string( context->isShutDown ) ).str( );
            }
            else if( type == 9 )
            {
                buf_send = getMessage( 18, std::to_string( static_cast< int >( context->statuses.getWorkerFromNum( std::stoul( message ) ).state ) ) ).str( );
            }
            else if( type == 10 )
            {
                buf_send = getMessage( 19, context->statuses.getWorkerFromNum( std::stoul( message ) ).name ).str( );
            }
            else
            {
                return;
            }


            auto self( this->shared_from_this( ));
            boost::asio::async_write( socket_, boost::asio::buffer( buf_send ),
                                      [ &, self ]( boost::system::error_code ec, std::size_t)
                                      {
                                          if ( !ec )
                                          {
                                              do_read( );
                                          }
                                      }
                                    );
        }

        void read_message( )
        {
            console::Log( ) << "Read message";
            auto self( this->shared_from_this( ) );
            data_.resize( size );

            socket_.async_read_some(
                    boost::asio::buffer( data_ ),
                    [ &, self ]( boost::system::error_code ec, std::size_t length )
                    {
                        if ( !ec )
                        {
                            message = data_;
                            console::Log( ) << "get Message: " << "type=" << type << " size=" << size << " mess=" << message;
                            send_message( );
                        }
                    } );

        }

        void
        do_write( )
        {
            auto self( this->shared_from_this( ));

            //std::string typeMessage( data_.begin( ), data_.begin( ) + 4 );
            //int type = std::stoi( typeMessage );

            //if ( type == 0 )
            {
                //data_ = std::to_string( 1 );
            }
            //else
            //    throw std::exception( );

            boost::asio::async_write( socket_, boost::asio::buffer( data_ ),
                                      [ &, self ]( boost::system::error_code ec, std::size_t)
                                      {
                                          if ( !ec )
                                          {
                                              do_read( );
                                          }
                                      }
                                    );
        }

        tcp::socket socket_;
        std::string data_;

        std::string buf_send;

        int type;
        int size;
        std::string message;
    };

    template< class _TypeTask >
    class server
    {
        std::shared_ptr< master_context< _TypeTask > > context;
    public:
        server( boost::asio::io_service &io_service, short port, std::shared_ptr< master_context< _TypeTask > > __context)
                : acceptor_( io_service, tcp::endpoint( tcp::v4( ), port )),
                  socket_( io_service )
        {
            do_accept( );
            context = __context;
        }

    private:
        void
        do_accept( )
        {
            acceptor_.async_accept(
                    socket_,
                    [ this ]( boost::system::error_code ec )
                    {
                        if ( !ec )
                        {
                            std::make_shared< session< _TypeTask > >( std::move( socket_ ), context )->start( );
                        }
                        do_accept( );
                    } );
        }

        tcp::acceptor acceptor_;
        tcp::socket socket_;
    };

    template< class _TypeTask >
    class gui_server
    {
        boost::asio::io_service io_service{ };
        server< _TypeTask > s;
        std::thread *thread_gui_server;
    public:
        gui_server( std::shared_ptr< master_context< _TypeTask > > __context, short port = 19920 ) :
                s( io_service, port, __context )
        {
            thread_gui_server = new std::thread(
                    [ & ]( )
                    {
                        io_service.run( );
                    } );
        }

        void stop( )
        {
            io_service.stop( );
        }

        ~gui_server( )
        {
            if ( thread_gui_server != nullptr )
            {
                thread_gui_server->join( );
            }
        }
    };

}
#endif //FTCL_GUI_SERVER_HPP
