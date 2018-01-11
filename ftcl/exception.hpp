#ifndef FTCL_EXCEPTION_HPP_INCLUDED
#define FTCL_EXCEPTION_HPP_INCLUDED

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
            return std::string{ textEx + " " + fileEx + " " + std::to_string( lineEx ) }.c_str( );
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

        class Illegal_size_recv_message :  public Exception
        {
        public:
            Illegal_size_recv_message(
                    std::string __fileEx,
                    std::size_t __lineEx
                ) : Exception(
                            std::string{ "Illegal size message" },
                            __fileEx,
                            __lineEx
                        ) { }
        };

        class Illegal_state_worker : public Exception
        {
            public:
            Illegal_state_worker(
                    std::string __fileEx,
                    std::size_t __lineEx
                ) : Exception(
                            std::string{ "Illegal worker state" },
                            __fileEx,
                            __lineEx
                        ) { }
        };

        class Illegal_rank : public Exception
        {
        public:
            Illegal_rank(
                    std::string __fileEx,
                    std::size_t __lineEx
                ) : Exception(
                            std::string{ "Illegal rank" },
                            __fileEx,
                            __lineEx
                        ) { }
        };

        class Error_worker_logger : public Exception
        {
        public:
            Error_worker_logger(
                    std::string __fileEx,
                    std::size_t __lineEx
                ) : Exception(
                            std::string{ "Logger can't send log message on master!" },
                            __fileEx,
                            __lineEx
                        ) { }
        };

        class Log_was_already_started : public Exception
        {
        public:
            Log_was_already_started(
                    std::string __fileEx,
                    std::size_t __lineEx
                ) : Exception(
                        std::string{ "Log was already started!" },
                        __fileEx,
                        __lineEx
                    ) { }
        };

        class Log_was_already_stopped : public Exception
        {
        public:
            Log_was_already_stopped(
                    std::string __fileEx,
                    std::size_t __lineEx
                ) : Exception(
                        std::string{ "Log was already stopped!" },
                        __fileEx,
                        __lineEx
                    ) { }
        };

        class Log_was_not_started : public Exception
        {
        public:
            Log_was_not_started(
                    std::string __fileEx,
                    std::size_t __lineEx
                ) : Exception(
                        std::string{ "Log was not started!" },
                        __fileEx,
                        __lineEx
                    ) { }
        };

        class Attempt_to_send_to_oneself : public Exception
        {
        public:
            Attempt_to_send_to_oneself(
                    std::string __fileEx,
                    std::size_t __lineEx
                ) : Exception(
                        std::string{ "Attempt to send to oneself!" },
                        __fileEx,
                        __lineEx
                    ) { }
        };

        class Attempt_to_check_to_oneself : public Exception
        {
        public:
            Attempt_to_check_to_oneself(
                    std::string __fileEx,
                    std::size_t __lineEx
                ) : Exception(
                        std::string{ "Attempt to check to oneself!" },
                        __fileEx,
                        __lineEx
                    ) { }
        };

        class Error_worker_shutDown : public Exception
        {
        public:
            Error_worker_shutDown(
                    std::string __fileEx,
                    std::size_t __lineEx
            ) : Exception(
                    std::string{ "Error worker shutdown" },
                    __fileEx,
                    __lineEx
                         ) { }
        };
    }
}

#endif // FTCL_EXCEPTION_HPP_INCLUDED
