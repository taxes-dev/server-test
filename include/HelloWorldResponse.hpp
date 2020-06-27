#pragma once
#include "Server.hpp"

class HelloWorldResponse
    : public Response
{
public:
    HelloWorldResponse(){};
    virtual void respond(SocketDescriptor socketd);
};