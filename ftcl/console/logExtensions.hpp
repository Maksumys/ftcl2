#ifndef _FTCL_CONSOLE_LOG_EXTENSIONS_HPP_INCLUDED
#define _FTCL_CONSOLE_LOG_EXTENSIONS_HPP_INCLUDED

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <utility>
#include <vector>
#include <list>


#include "ftcl/queue.hpp"
#include "ftcl/multithread/queue.hpp"

namespace ftcl::console::extensions
{

    namespace Color
    {
        [[maybe_unused]]constexpr auto RESET = "\033[0m";
        [[maybe_unused]]constexpr auto BLACK = "\033[30m";              /// Black
        [[maybe_unused]]constexpr auto RED = "\033[31m";                /// Red
        [[maybe_unused]]constexpr auto GREEN = "\033[32m";              /// Green
        [[maybe_unused]]constexpr auto YELLOW = "\033[33m";             /// Yellow
        [[maybe_unused]]constexpr auto BLUE = "\033[34m";               /// Blue
        [[maybe_unused]]constexpr auto MAGENTA = "\033[35m";            /// Magenta
        [[maybe_unused]]constexpr auto CYAN = "\033[36m";               /// Cyan
        [[maybe_unused]]constexpr auto WHITE = "\033[37m";              /// White
        [[maybe_unused]]constexpr auto BOLDBLACK = "\033[1m\033[30m";   /// Bold Black
        [[maybe_unused]]constexpr auto BOLDRED = "\033[1m\033[31m";     /// Bold Red
        [[maybe_unused]]constexpr auto BOLDGREEN = "\033[1m\033[32m";   /// Bold Green
        [[maybe_unused]]constexpr auto BOLDYELLOW = "\033[1m\033[33m";  /// Bold Yellow
        [[maybe_unused]]constexpr auto BOLDBLUE = "\033[1m\033[34m";    /// Bold Blue
        [[maybe_unused]]constexpr auto BOLDMAGENTA = "\033[1m\033[35m"; /// Bold Magenta
        [[maybe_unused]]constexpr auto BOLDCYAN = "\033[1m\033[36m";    /// Bold Cyan
        [[maybe_unused]]constexpr auto BOLDWHITE = "\033[1m\033[37m";   /// Bold White
    }

    enum class Level
    {
        Error, Info, Warning, Debug1, Debug2
    };

    /*!
     * class Precission
     */
    class Precission
    {
    public:
        explicit Precission( std::string __delimiter ) : delimiter( std::move( __delimiter ) ) { }
        std::string delimiter { "" };
    };

    /*!
     * class LogMessage
     * Тип сообщения пересылки
     */
    class LogMessage
    {
    public:
        Level level{ Level::Info };
        std::string time;
        std::string color;
        std::string stream;
        std::size_t numNode;
        std::string delimiter;
    };
}

#endif // _FTCL_CONSOLE_LOG_EXTENSIONS_HPP_INCLUDED
