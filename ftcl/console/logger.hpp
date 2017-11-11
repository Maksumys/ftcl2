#ifndef _FTCL_CONSOLE_LOGGER_HPP_INCLUDED
#define _FTCL_CONSOLE_LOGGER_HPP_INCLUDED

#ifdef FTCL_MPI_INCLUDED
#include "ftcl/console/loggerMpi.hpp"
namespace ftcl::console
{
    using Logger = _ftcl::console::LoggerMpi;
}
#else
#include "ftcl/console/loggerCommon.hpp"
namespace ftcl::console
{
    using Logger = _ftcl::console::LoggerCommon;
}

#endif
#endif
