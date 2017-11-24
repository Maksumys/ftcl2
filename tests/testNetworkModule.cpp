#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "ftcl/network.hpp"
#include "ftcl/console/log.hpp"


TEST(NetworkModule, init0)
{
    auto &network = ftcl::NetworkModule::Instance( );
    EXPECT_EQ( network.getName( ), std::string{ "Air-Maksim" } );
}

TEST(NetworkModule, init)
{
    auto &network = ftcl::NetworkModule::Instance( );
    if( network.getRank( ) == 0 )
    {
        std::string str;
        str = "HELLO!\n";
        network.send( str, 1, ftcl::TypeMessage::MessageLog );
    }
    else if( network.getRank( ) == 1 )
    {
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
        auto check = network.checkMessage( MPI_ANY_SOURCE, ftcl::TypeMessage::MessageLog );
        if( std::get< 0 >( check ) )
        {
            auto str = network.getMessage( std::get< 1 >( check ) );
            EXPECT_EQ( "HELLO!\n", str.data( ) );
        }
    }
}


int main( int argc, char *argv[ ] )
{
    ::testing::InitGoogleTest ( &argc, argv );
    return RUN_ALL_TESTS();
}
