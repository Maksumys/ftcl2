#ifndef _NETWORK_HPP_INCLUDED
#define _NETWORK_HPP_INCLUDED

/*!
 *  \brief Модуль передачи данных
 *   не правильно работает, если инициализировать в другом singleton'е
 *   при определенных условиях
 */

#include <mpi.h>
#include <iostream>

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
private:
    ~NetworkModule( );
};

#endif // _NETWORK_HPP_INCLUDED