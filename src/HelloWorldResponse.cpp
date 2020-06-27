#include "HelloWorldResponse.hpp"
#include <iostream>

void HelloWorldResponse::respond(SocketDescriptor socketd)
{
    std::string message{"Hello, world!\n\nBye.\n"};
    int written = write(socketd.get(), message.c_str(), message.length());

    if (written < 0)
    {
        std::cout << "ERROR writing to socket" << std::endl;
    }
}
