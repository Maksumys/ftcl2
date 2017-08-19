#ifndef _FTCL_QUEUE_HPP_INCLUDED
#define _FTCL_QUEUE_HPP_INCLUDED

#include <memory>
#include <ftcl/exception.hpp>

namespace ftcl
{
    template< class _T >
    class queue
    {
    public:
        using type = _T;
        using size_type = std::size_t;
    protected:
        std::unique_ptr< type[ ] > mas_queue;
        size_type maxSize;
        size_type head;
        size_type tail;
        size_type distance;
    public:
        explicit queue( ) noexcept;
        explicit queue( const size_type n );
        void reserve( const size_type n );

        bool empty( ) const noexcept;
        size_type size( ) const noexcept;
        size_type getMaxSize( ) const noexcept;

        void push( const type &elem );
        void pop( );
        type back( ) const;
        type front( ) const;
    };

    template< class _T >
    queue< _T >::queue( ) noexcept :
        maxSize{ 0 }, head{ 0 }, tail{ 0 }, distance{ 0 }
    {
    }

    template< class _T >
    queue< _T >::queue( const queue< _T >::size_type __n ) :
        mas_queue{ new type[ __n ] },
        maxSize{ __n },
        head{ 0 },
        tail{ 0 },
        distance{ __n }
    {
    }

    template< class _T >
    void
    queue< _T >::reserve( const queue< _T >::size_type __n )
    {
        mas_queue.reset( new type[ __n ] );
        maxSize = __n;
        head = 0;
        tail = 0;
        distance = __n;
    }

    template< class _T >
    bool
    queue< _T >::empty( ) const noexcept
    {
        return maxSize == distance;
    }

    template< class _T >
    void
    queue< _T >::push( const type &__elem )
    {
        if ( distance > 0 )
        {
            mas_queue[ head ] = __elem;
            ( head == maxSize - 1 ) ? head = 0 : head++;
            distance--;
        }
        else
            throw ftcl::exception::Queue_overflow( __FILE__, __LINE__ );
    }

    template< class _T >
    void
    queue< _T >::pop( )
    {
        if( distance != maxSize )
        {
            ( tail == maxSize - 1 ) ? tail = 0 : tail++;
            distance++;
        }
        else
            throw ftcl::exception::Queue_empty( __FILE__, __LINE__ );
    }

    template< class _T >
    _T
    queue< _T >::back( ) const
    {
        if( !empty( ) )
            return mas_queue[ tail ];
        else
            throw ftcl::exception::Queue_empty( __FILE__, __LINE__ );
    }

    template< class _T >
    _T
    queue< _T >::front( ) const
    {
        if( !empty( ) )
            if( head == 0 )
                return mas_queue[ maxSize - 1 ];
            else
                return mas_queue[ head - 1 ];
        else
            throw ftcl::exception::Queue_empty( __FILE__, __LINE__ );
    }

    template< class _T >
    typename queue< _T >::size_type
    queue< _T >::getMaxSize( ) const noexcept
    {
        return maxSize;
    }

    template< class _T >
    typename queue< _T >::size_type
    queue< _T >::size( ) const noexcept
    {
        return maxSize - distance;
    }

}

#endif // _FTCL_QUEUE_HPP_INCLUDED
