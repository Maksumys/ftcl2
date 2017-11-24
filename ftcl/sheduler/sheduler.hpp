#ifndef _FTCL_SHEDULER_HPP_INCLUDED
#define _FTCL_SHEDULER_HPP_INCLUDED

#include "ftcl/console/log.hpp"

namespace ftcl
{
    class Sheduler
    {
    public:
        Sheduler( ) = default;
        virtual ~Sheduler( ) = default;

        virtual void run( ) = 0;
        virtual void initialize( ) = 0;
        virtual void finalize( ) = 0;

        Sheduler( Sheduler& ) = delete;
        Sheduler& operator=( const Sheduler& ) = delete;
    };
}

#endif // _FTCL_SHEDULER_HPP_INCLUDED
