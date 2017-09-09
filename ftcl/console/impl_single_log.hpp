#ifndef _FTCL_CONSOLE_IMPL_SINGLE_LOG_HPP_INCLUDED
#define _FTCL_CONSOLE_IMPL_SINGLE_LOG_HPP_INCLUDED

#include "ftcl/queue.hpp"

namespace ftcl { namespace console {

    class implSingleLog
    {
    public:
        queue< std::string > queueStream{ 100 };
        
    };

} }

#endif // _FTCL_CONSOLE_IMPL_SINGLE_LOG_HPP_INCLUDED
