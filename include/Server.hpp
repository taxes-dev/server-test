#pragma once
#include <netinet/in.h>
#include <string>
#include <unistd.h>

class SocketDescriptor
{
public:
    SocketDescriptor() : socket_fd{INVALID} {};
    SocketDescriptor(const int &socket_fd) : socket_fd{socket_fd} {};
    SocketDescriptor(const SocketDescriptor &) = delete;
    SocketDescriptor(SocketDescriptor &&other) : socket_fd{other.socket_fd}
    {
        other.socket_fd = INVALID;
    }
    ~SocketDescriptor()
    {
        if (valid())
        {
            close(socket_fd);
            socket_fd = INVALID;
        }
    }
    int get() const
    {
        return socket_fd;
    }
    bool valid() const
    {
        return socket_fd > INVALID;
    }
    SocketDescriptor &operator=(SocketDescriptor &&other)
    {
        socket_fd = other.socket_fd;
        other.socket_fd = INVALID;
        return *this;
    }

private:
    const int INVALID = -1;
    int socket_fd;
};

class Response
{
public:
    Response(){};
    virtual ~Response(){};

    virtual void respond(SocketDescriptor socketd) = 0;
};

class Server
{
public:
    Server(const std::string &bind_addr, const int &bind_port);
    ~Server(){};
    void listen(const int &max_connections, Response &response);

private:
    void init_ip4(const in_addr &addr, const int &port);
    void init_ip6(const in6_addr &addr, const int &port);
    SocketDescriptor socketd;
    bool is_ip6;
};
