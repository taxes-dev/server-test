#pragma once
#include "Server.hpp"

class EchoResponse
    : public Response
{
public:
    EchoResponse(){};
    virtual void respond(SocketDescriptor socketd);
};