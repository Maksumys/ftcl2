#ifndef _FTCL_CHECKPOINT_HPP_ENABLED
#define _FTCL_CHECKPOINT_HPP_ENABLED

#include <string>
#include <sstream>
#include <fstream>

#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>

#include "ftcl/sheduler/sheduler.hpp"

/*
 * TODO: ДОБАВИТЬ к каждой задаче ее номер например с помощью std::tuple, это нужно для того чтобы знать какие задачи
 * записаны в чекпоинт!!!!!!!!!! чтобы потом их убрать из списка нерешенных задач
 */


namespace out
{
    class Stream
    {
    protected:
        boost::iostreams::basic_array_source<char> device;
        boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s;
        boost::archive::binary_iarchive ia;
    public:
        Stream( const std::string &str ) : device{ str.data( ), str.size( ) }, s{ device }, ia{ s }
        {
        }

        template< typename _T >
        Stream& operator>>( _T &__t )
        {
            ia >> __t;
            return *this;
        }
    };
}

namespace in
{
    class Stream
    {
    protected:
        std::string serial_str;
        boost::iostreams::back_insert_device<std::string> inserter;
        boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s;
        boost::archive::binary_oarchive oa;
    public:
        Stream( ) : inserter{ serial_str }, s{ inserter }, oa{ s }
        {
        }

        template< typename _T >
        Stream& operator<<( _T &__t )
        {
            oa << __t;
            s.flush( );
            return *this;
        }

        std::string str( )
        {
            return serial_str;
        }
    };
}

template< class _Task, typename = typename std::enable_if< std::is_default_constructible< _Task >::value >::type >
class CheckPoint
{
protected:
    std::string     file_name;
    std::string     file_checkpoint_data;
    std::ifstream   file_stream;
    std::ofstream   file_stream_out;

public:
    CheckPoint( ) = default;

    CheckPoint( const std::string &__file_name ) : file_name( __file_name )
    {
    }

    void setName( const std::string &__file_name )
    {
        file_name = __file_name;
    }

    bool findCheckPoint( )
    {
        if( file_name.empty() )
            throw std::exception( );
        std::ifstream test( file_name );
        return test.is_open( );
    }

    void open( )
    {
        file_stream.open( file_name );
    }

    std::vector< _Task > load( std::shared_ptr< ftcl::master_context< _Task > > context )
    {
        file_stream.open( file_name );
        if( !file_stream )
            throw std::exception( );

        boost::archive::binary_iarchive oa( file_stream );

        _Task t;
        std::uint64_t num_task;
        std::vector< _Task > tasks;

        while( !file_stream.eof( ) )
        {
            oa >> t;
            oa >> num_task;
            tasks.push_back( t );
        }
        file_stream.close( );
    }

    void save( const _Task &__task, const std::uint64_t __num_task )
    {
        file_stream_out.open( file_name );
        in::Stream stream;
        stream << __task;
        stream << __num_task;
        file_stream_out << stream.str( );
        file_stream_out.close( );
    }

    //_Task load( )
    //{
        //out::Stream stream( file_stream. );
        //_Task task;
        //file_stream >> stream;
        //stream >> task;
        //return task;
    //}

};

#endif
