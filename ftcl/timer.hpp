//
// Created by Максим Кузин on 08.01.2018.
//

#ifndef FTCL_TIMER_HPP
#define FTCL_TIMER_HPP

#include <chrono>

namespace ftcl
{
    /*!
 * Timer - класс для засечения интервалов времени
 */
    class Timer
    {
    protected:
        std::chrono::steady_clock::time_point startTime;        ///< начало отсчета интервала
    public:
        /*!
         * Начать отсчет
         */
        void start( )
        {
            startTime = std::chrono::steady_clock::now( );
        }

        /*!
         * Закончить отсчет
         * @return время интервала
         */
        std::int64_t end( )
        {
            auto endTime = std::chrono::steady_clock::now( );
            return std::chrono::duration_cast< std::chrono::milliseconds >( endTime - startTime ).count( );
        }
    };
}

#endif //FTCL_TIMER_HPP
