#define MPI_INCLUDED
#ifdef MPI_INCLUDED
#ifndef _FTCL_SHEDULER_SHEDULER_HPP_INCLUDED
#define _FTCL_SHEDULER_SHEDULER_HPP_INCLUDED

#include "ftcl/console/log.hpp"

namespace ftcl
{
    class Sheduler
    {
    public:
        Sheduler( ) = default;

        virtual void run( ) = 0;

        Sheduler( Sheduler& ) = delete;
        Sheduler& operator=( const Sheduler& ) = delete;
    };
}

#endif // _FTCL_SHEDULER_SHEDULER_HPP_INCLUDED
#endif // MPI_INCLUDED
