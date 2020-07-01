#include <cstdlib>
#include <iostream>
#include "Server.hpp"
#include "EchoResponse.hpp"

void usage()
{
    std::cout << "Usage:\n\nServerTest [ip] [port]\n\n\tip - The IP address to bind to.\n\tport - The TCP port to use.\n\n"
              << std::endl;
    exit(1);
}

int main(int argc, char **argv)
{
    std::cout << "ServerTest starting ..." << std::endl;
    if (argc < 3)
    {
        usage();
    }
    std::string bind_addr{argv[1]};
    if (bind_addr.length() < 1)
    {
        usage();
    }
    int bind_port{static_cast<int>(strtol(argv[2], nullptr, 10))};
    if (bind_port < 1)
    {
        usage();
    }

    try
    {

        Server server{bind_addr, bind_port};
        std::cout << "\nListening for connections ..." << std::endl;
        int max_connections{5};
        EchoResponse response{};
        server.listen(max_connections, response);
    }
    catch (std::runtime_error &e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
