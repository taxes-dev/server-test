#include <arpa/inet.h>
#include <ifaddrs.h>
#include <iostream>
#include <memory>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include "Server.hpp"

Server::Server(const std::string &bind_addr, const int &bind_port)
{
    /* Determine IP address to bind to */
    if (bind_addr == "-")
    {
        /* Get the first non-loopback interface */
        ifaddrs *ifap_addr;
        if (getifaddrs(&ifap_addr) == 0)
        {
            auto ifap = std::unique_ptr<ifaddrs, void (*)(ifaddrs *)>(ifap_addr, &freeifaddrs);
            ifaddrs *ifap_next = ifap.get();
            while (ifap_next)
            {
                if (ifap_next->ifa_addr &&
                    ifap_next->ifa_addr->sa_family == AF_INET &&
                    (ifap_next->ifa_flags & IFF_LOOPBACK) == 0)
                {
                    auto ifa4 = reinterpret_cast<sockaddr_in *>(ifap_next->ifa_addr);
                    init_ip4(ifa4->sin_addr, bind_port);
                    break;
                }
                else if (ifap_next->ifa_addr &&
                         ifap_next->ifa_addr->sa_family == AF_INET6 &&
                         (ifap_next->ifa_flags & IFF_LOOPBACK) == 0)
                {
                    auto ifa6 = reinterpret_cast<sockaddr_in6 *>(ifap_next->ifa_addr);
                    init_ip6(ifa6->sin6_addr, bind_port);
                    break;
                }
                ifap_next = ifap_next->ifa_next;
            }
        }
        else
        {
            throw std::runtime_error("Unable to get interface addresses");
        }
    }
    else
    {

        /* See if the provided address is IPv4 */
        in_addr addr_ip4{};
        if (inet_pton(AF_INET, bind_addr.c_str(), &addr_ip4) == 1)
        {
            init_ip4(addr_ip4, bind_port);
        }
        else
        {
            /* See if the provided address is IPv6 */
            in6_addr addr_ip6{};
            if (inet_pton(AF_INET6, bind_addr.c_str(), &addr_ip6) == 1)
            {
                init_ip6(addr_ip6, bind_port);
            }
            else
            {
                throw std::runtime_error("Invalid inet address");
            }
        }
    }
}

void Server::init_ip4(const in_addr &addr, const int &port)
{
    char buffer[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &addr, reinterpret_cast<char *>(buffer), sizeof(buffer)))
    {
        std::cout << "Binding to " << buffer << ":" << port << " ..." << std::endl;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = addr;

    /* Open socket */
    int socket_fd = socket(serv_addr.sin_family, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        throw std::runtime_error("Error opening socket");
    }
    socketd = SocketDescriptor{socket_fd};

    /* Bind socket to address */
    if (bind(socketd.get(), reinterpret_cast<const sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0)
    {
        throw std::runtime_error("Error on binding");
    }

    is_ip6 = false;
}

void Server::init_ip6(const in6_addr &addr, const int &port)
{
    char buffer[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET6, &addr, reinterpret_cast<char *>(buffer), sizeof(buffer)))
    {
        std::cout << "Binding to " << buffer << ":" << port << " ..." << std::endl;
    }

    sockaddr_in6 serv_addr{};
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(port);
    serv_addr.sin6_addr = addr;

    /* Open socket */
    int socket_fd = socket(serv_addr.sin6_family, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        throw std::runtime_error("Error opening socket");
    }
    socketd = SocketDescriptor{socket_fd};

    /* Allow only IPv6 connections (blocks IPv4-mapped addresses) */
    int opt = 1;
    if (setsockopt(socketd.get(), IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) < 0)
    {
        throw std::runtime_error("Unable to set IPV6_V6ONLY");
    }

    /* Bind socket to address */
    if (bind(socketd.get(), reinterpret_cast<const sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0)
    {
        throw std::runtime_error("Error on binding");
    }

    is_ip6 = true;
}

void Server::listen(const int &max_connections, Response &response)
{
    /* Start listening for client connections */
    ::listen(socketd.get(), max_connections);
    sockaddr_storage cli_addr{};
    size_t clilen{sizeof(cli_addr)};
    char buffer[INET6_ADDRSTRLEN];
    while (1)
    {
        int client_socket_fd = accept(socketd.get(), reinterpret_cast<sockaddr *>(&cli_addr), reinterpret_cast<socklen_t *>(&clilen));
        if (client_socket_fd < 0)
        {
            std::cout << "Error on accept " << errno << std::endl;
            continue;
        }
        SocketDescriptor clientd{client_socket_fd};

        /* Status */
        std::cout << "Connection from: ";
        if (is_ip6)
        {
            auto cli_addr_in6 = reinterpret_cast<sockaddr_in6 *>(&cli_addr);
            if (inet_ntop(AF_INET6, &cli_addr_in6->sin6_addr, reinterpret_cast<char *>(buffer), sizeof(buffer)))
            {
                std::cout << buffer << ":" << cli_addr_in6->sin6_port;
            }
        }
        else
        {
            auto cli_addr_in4 = reinterpret_cast<sockaddr_in *>(&cli_addr);
            if (inet_ntop(AF_INET, &cli_addr_in4->sin_addr, reinterpret_cast<char *>(buffer), sizeof(buffer)))
            {
                std::cout << buffer << ":" << cli_addr_in4->sin_port;
            }
        }
        std::cout << std::endl;

        /* Create child process */
        pid_t pid = fork();

        if (pid < 0)
        {
            std::cout << "Error on fork " << errno << std::endl;
            continue;
        }

        if (pid == 0)
        {
            /* Respond to connected client */
            response.respond(std::move(clientd));
            return;
        }
    }
}