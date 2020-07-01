#include "EchoResponse.hpp"
#include <iostream>
#include <sys/socket.h>
#include <stdexcept>

#define BUF_SIZE 60
const std::string BANNER_IN{"Type what you want, and it will be echoed. Ctrl-D to end.\n\n"};
const std::string BANNER_OUT{"\nBye.\n\n"};

void socket_write(const SocketDescriptor &socketd, const char *buffer, const ssize_t &bufsize)
{
    ssize_t total{0};
    while (total < bufsize)
    {
        auto written = write(socketd.get(), buffer + total, bufsize - total);
        if (written < 0)
        {
            throw std::runtime_error("Error writing to socket");
        }
        total += written;
    }
}

void EchoResponse::respond(SocketDescriptor socketd)
{
    char buffer[BUF_SIZE];
    bool done{false};

    socket_write(socketd, BANNER_IN.c_str(), BANNER_IN.length());

    while (1)
    {
        auto bytes = read(socketd.get(), &buffer, sizeof(buffer));
        if (bytes > 0)
        {
            for (int i = 0; i < bytes; i++)
            {
                if (buffer[i] == '\x04')
                {
                    buffer[i] = ' ';
                    done = true;
                    break;
                }
            }

            socket_write(socketd, static_cast<char *>(buffer), bytes);

            if (done)
            {
                socket_write(socketd, BANNER_OUT.c_str(), BANNER_OUT.length());
                break;
            }
        }
        else if (bytes < 0)
        {
            throw std::runtime_error("Error reading from socket");
        }
    }
    shutdown(socketd.get(), SHUT_RDWR);
}
