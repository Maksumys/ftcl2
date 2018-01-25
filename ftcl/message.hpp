#ifndef FTCL_MESSAGE_HPP
#define FTCL_MESSAGE_HPP

#include <iostream>

namespace ftcl
{
    class message
    {
    public:
        int fromRank;
        int toRank;
    };


    class message0 : public message
    {
    public:
        bool q;

        friend std::istream& operator>>( std::istream& stream, message0 &msg )
        {
            stream >> msg.q;
            return stream;
        }

        friend std::ostream& operator<<( std::ostream &stream, const message0 &msg )
        {
            stream << msg.q;
            return stream;
        }
    };

    class message1 : public message
    {
    public:
        int q;

        friend std::istream& operator>>( std::istream& stream, message1 &msg )
        {
            stream >> msg.q;
            return stream;
        }

        friend std::ostream& operator<<( std::ostream &stream, const message1 &msg )
        {
            stream << msg.q;
            return stream;
        }
    };

    class message2 : public message
    {
    public:
        std::string str;

        friend std::istream& operator>>( std::istream& stream, message2 &msg )
        {
            stream >> msg.str;
            return stream;
        }

        friend std::ostream& operator<<( std::ostream &stream, const message2 &msg )
        {
            stream << msg.str;
            return stream;
        }
    };
}

#endif //FTCL_MESSAGE_HPP
