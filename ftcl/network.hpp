#ifdef FTCL_MPI_INCLUDED
#ifndef _NETWORK_HPP_INCLUDED
#define _NETWORK_HPP_INCLUDED

/*!
 *  \brief Модуль передачи данных
 *   не правильно работает, если инициализировать в другом singleton'е
 *   при определенных условиях
 */

#include "ftcl/exception.hpp"

#include <cstdint>
#include <mpi.h>
#include <iostream>
#include <vector>
#include <sstream>

namespace ftcl
{
    enum class TypeMessage
    {
        MessageLog, MessageLogExit
    };


    class NetworkModule
    {
    public:
        using Number = int;

    private:
        Number rank;
        Number size;

        NetworkModule( );
    public:
        static NetworkModule& Instance( );
        Number getSize( ) const noexcept;
        Number getRank( ) const noexcept;
        bool master( ) const noexcept;

        MPI_Request
        send(
                const std::string &__data,
                const std::uint64_t __toRank,
                const TypeMessage __typeMessage
            ) const;


        /*!
         * \brief checkMessage
         * \param source ( -1 значит MPI_ANY_SOURCE )
         * \param typeMessage
         * \return
         */
        std::tuple< bool, MPI_Status >
        checkMessage(
                const int source,
                const TypeMessage typeMessage
            );

        std::vector< char >
        getMessage(
                const MPI_Status &status
            );
    private:
        ~NetworkModule( );
    };

}

#endif // _NETWORK_HPP_INCLUDED
#endif // MPI_INCLUDED
