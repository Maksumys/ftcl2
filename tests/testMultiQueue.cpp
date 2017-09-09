#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include "ftcl/multithread/queue.hpp"

TEST( Queue, empty )
{
    ftcl::multithread::queue< int > queue( 5 );
    EXPECT_EQ( queue.size( ), 0 );
    EXPECT_TRUE( queue.empty( ) );
    queue.push( 1 );
    EXPECT_EQ( queue.size( ), 1 );
    EXPECT_FALSE( queue.empty( ) );
    queue.pop( );
    EXPECT_TRUE( queue.empty( ) );
    EXPECT_EQ( queue.size( ), 0 );
}

TEST( Queue, push )
{
    ftcl::multithread::queue< int > queue( 5 );
    EXPECT_THROW( queue.back( ), ftcl::exception::Queue_empty );
    EXPECT_THROW( queue.front( ), ftcl::exception::Queue_empty );
    queue.push( 1 );
    EXPECT_EQ( queue.back( ), 1 );
    EXPECT_EQ( queue.front( ), 1 );
    queue.push( 2 );
    EXPECT_EQ( queue.back( ), 1 );
    EXPECT_EQ( queue.front( ), 2 );
    queue.push( 3 );
    EXPECT_EQ( queue.back( ), 1 );
    EXPECT_EQ( queue.front( ), 3 );
    queue.push( 4 );
    EXPECT_EQ( queue.back( ), 1 );
    EXPECT_EQ( queue.front( ), 4 );
    queue.push( 5 );
    EXPECT_EQ( queue.back( ), 1 );
    EXPECT_EQ( queue.front( ), 5 );
    EXPECT_EQ( queue.back( ), 1 );
    EXPECT_EQ( queue.front( ), 5 );
    queue.pop( );
    EXPECT_EQ( queue.back( ), 2 );
    EXPECT_EQ( queue.front( ), 5 );
    queue.push( 6 );
    EXPECT_EQ( queue.back( ), 2 );
    EXPECT_EQ( queue.front( ), 6 );
    EXPECT_EQ( queue.back( ), 2 );
    EXPECT_EQ( queue.front( ), 6 );
    queue.pop( );
    EXPECT_EQ( queue.back( ), 3 );
    EXPECT_EQ( queue.front( ), 6 );
    queue.pop( );
    EXPECT_EQ( queue.back( ), 4 );
    EXPECT_EQ( queue.front( ), 6 );
    queue.pop( );
    EXPECT_EQ( queue.back( ), 5 );
    EXPECT_EQ( queue.front( ), 6 );
    queue.pop( );
    EXPECT_EQ( queue.back( ), 6 );
    EXPECT_EQ( queue.front( ), 6 );
    queue.pop( );
    EXPECT_THROW( queue.back( ), ftcl::exception::Queue_empty );
    EXPECT_THROW( queue.front( ), ftcl::exception::Queue_empty );
    queue.push( 7 );
    EXPECT_EQ( queue.back( ), 7 );
    EXPECT_EQ( queue.front( ), 7 );
    queue.push( 8 );
    EXPECT_EQ( queue.back( ), 7 );
    EXPECT_EQ( queue.front( ), 8 );
    queue.push( 9 );
    EXPECT_EQ( queue.back( ), 7 );
    EXPECT_EQ( queue.front( ), 9 );
    queue.push( 10 );
    EXPECT_EQ( queue.back( ), 7 );
    EXPECT_EQ( queue.front( ), 10 );
    queue.push( 11 );
    EXPECT_EQ( queue.back( ), 7 );
    EXPECT_EQ( queue.front( ), 11 );
    queue.pop( );
    EXPECT_EQ( queue.back( ), 8 );
    EXPECT_EQ( queue.front( ), 11 );
    queue.pop( );
    EXPECT_EQ( queue.back( ), 9 );
    EXPECT_EQ( queue.front( ), 11 );
    queue.pop( );
    EXPECT_EQ( queue.back( ), 10 );
    EXPECT_EQ( queue.front( ), 11 );
    queue.pop( );
    EXPECT_EQ( queue.back( ), 11 );
    EXPECT_EQ( queue.front( ), 11 );
    queue.push( 12 );
    EXPECT_EQ( queue.back( ), 11 );
    EXPECT_EQ( queue.front( ), 12 );
    queue.push( 13 );
    EXPECT_EQ( queue.back( ), 11 );
    EXPECT_EQ( queue.front( ), 13 );
    queue.push( 14 );
    EXPECT_EQ( queue.back( ), 11 );
    EXPECT_EQ( queue.front( ), 14 );
    queue.push( 15 );
    EXPECT_EQ( queue.back( ), 11 );
    EXPECT_EQ( queue.front( ), 15 );
}

TEST( Queue, pushMulti )
{
    ftcl::multithread::queue< int > queue( 100 );
    std::vector< std::thread > threads;
    threads.push_back( std::thread(
            [ &queue ]( )
            {
                for( int i = 0; i < 100000; i++ )
                    queue.push( i );
            }
        ) );

    threads.push_back( std::thread(
            [ &queue ]( )
            {
                for( int i = 0; i < 100000; i++ )
                {
                    while( true )
                        if( !queue.empty( ) )
                        {
                            EXPECT_EQ( queue.back( ), i );
                            break;
                        }
                    queue.pop( );
                }
            }
        ) );

    for( auto &elem : threads )
        elem.join( );
}

TEST( MultiQueue, empty )
{
    ftcl::multithread::queue< int > queue( 100 );
    EXPECT_TRUE( queue.empty( ) );
    queue.push( 10 );
    EXPECT_FALSE( queue.empty( ) );
    queue.pop( );
    EXPECT_TRUE( queue.empty( ) );
}

int main( int argc, char *argv[ ] )
{
    ::testing::InitGoogleTest ( &argc, argv );
    return RUN_ALL_TESTS( );
}
