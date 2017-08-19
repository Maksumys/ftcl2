#ifndef _FTCL_EXCEPTION_HPP_INCLUDED
#define _FTCL_EXCEPTION_HPP_INCLUDED

#include <exception>
#include <string>

namespace ftcl
{
    class Exception : public std::exception
    {
    protected:
        std::string textEx;         ///< Текст ошибки
        std::string fileEx;         ///< Файл в котором произошло исключение
        std::size_t lineEx;         ///< Строка в которой произошло исключение
    public:
        Exception(
                std::string __textEx,
                std::string __fileEx,
                std::size_t __lineEx
            ) : textEx( __textEx ), fileEx( __fileEx ), lineEx( __lineEx ) { }

        const char* what( ) const noexcept override
        {
            return textEx.c_str( );
        }
    };

    namespace exception
    {
        class Queue_overflow : public Exception
        {
        public:
            Queue_overflow(
                    std::string __fileEx, std::size_t __lineEx
                ) : Exception( std::string{ "Queue is overflow" }, __fileEx, __lineEx ) { }

        };

        class Queue_empty : public Exception
        {
        public:
            Queue_empty(
                std::string __fileEx, std::size_t __lineEx
            ) : Exception( std::string{ "Queue is empty" }, __fileEx, __lineEx ) { }
        };
    }
}

#endif // _FTCL_EXCEPTION_HPP_INCLUDED
