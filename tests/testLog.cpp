#include "ftcl/console/log.hpp"
#include "ftcl/console/logger.hpp"
#include <gtest/gtest.h>

using namespace ftcl::console;

TEST( Log, output )
{
    {
        Logger::Instance( ).disableOutputTime( );
        std::stringstream buffer;
        std::streambuf *sbuf = std::cout.rdbuf();
        std::cout.rdbuf(buffer.rdbuf());
        Log( ) << "Hello World";
        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
        std::cout.rdbuf(sbuf);
        EXPECT_EQ( buffer.str( ), "Hello World\n" );
    }

    {
        std::stringstream buffer;
        std::streambuf *sbuf = std::cout.rdbuf();
        std::cout.rdbuf(buffer.rdbuf());
        Log( ) << "World Hello";
        Log( ) << "World Hello";
        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
        std::cout.rdbuf(sbuf);
        EXPECT_EQ( buffer.str( ), "World Hello\nWorld Hello\n" );
    }
}

int main( int argc, char *argv[ ] )
{
    ::testing::InitGoogleTest ( &argc, argv );
    return RUN_ALL_TESTS( );
}
