#ifndef _FTCL_MULTITHREAD_QUEUE_HPP_INCLUDED
#define _FTCL_MULTITHREAD_QUEUE_HPP_INCLUDED

#include "ftcl/queue.hpp"
#include <mutex>
#include <condition_variable>

namespace ftcl { namespace multithread {

    //потокобезопасная очередь с одним мьютексом
    template<class _T >
    class queue : public ftcl::queue< _T >
    {
    public:
        using base_queue = ftcl::queue< _T >;
        using type = typename base_queue::type;
        using size_type = typename base_queue::size_type;
        using mutex_type = std::mutex;
    protected:
        mutex_type mutex_lock;
        std::condition_variable condition_head;
        std::condition_variable condition_tail;
        bool isClearAction;
    public:
        queue( ) noexcept : base_queue( ), isClearAction { false }
        {
        }

        explicit queue( const size_type __n ) : base_queue( __n ), isClearAction{ false }
        {
        }

        void push( const _T &__elem )
        {
            std::unique_lock< mutex_type > tail_lock( mutex_lock );
            condition_head.wait( tail_lock,
            [ & ]
            {
                if( !isClearAction )
                {
                    if( base_queue::distance > 0 )
                    {
                        return true;
                    }
                    condition_tail.notify_one( );
                    return false;
                }
                else
                    return true;
            } );
            if( !isClearAction )
            {
                base_queue::push( __elem );
                condition_tail.notify_one( );
            }
        }

        bool try_push( const _T &__elem )
        {
            if( mutex_lock.try_lock( ) )
                if( base_queue::distance > 0 )
                {
                    base_queue::push( __elem );
                    mutex_lock.unlock();
                    return true;
                }
                mutex_lock.unlock();
            return false;
        }

        void pop( )
        {
            std::unique_lock< mutex_type > head_lock( mutex_lock );
            condition_tail.wait( head_lock,
            [ & ]
            {
                if( !isClearAction )
                {
                    if( base_queue::distance != base_queue::maxSize )
                        return true;
                    condition_head.notify_one( );
                    return false;
                }
                else
                    return true;
            } );
            if( !isClearAction )
            {
                base_queue::pop( );
                condition_head.notify_one();
            }
        }

        bool try_pop( )
        {
            if( mutex_lock.try_lock( ) )
                if ( base_queue::distance != base_queue::maxSize )
                {
                    base_queue::pop( );
                    mutex_lock.unlock();
                    return true;
                }
            return false;
        }

        _T& front( )
        {
            std::lock_guard< mutex_type > lock( mutex_lock );
            return base_queue::front( );
        }

        _T& back( )
        {
            std::lock_guard< mutex_type > lock( mutex_lock );
            return base_queue::back( );
        }

        std::size_t getDistance( )
        {
            std::lock_guard< mutex_type > lock( mutex_lock );
            return base_queue::getDistance( );
        }
    };


} }

#endif //_FTCL_MULTITHREAD_QUEUE_HPP_INCLUDED
