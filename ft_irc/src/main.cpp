

// int main(int argc, char **argv) 
// {
//     if (argc != 3) 
//     {
//         std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
//         return 1;
//     }
//     int port = std::atoi(argv[1]);
//     std::string password = argv[2];

//     Server server(port, password);
//     server.start();

//     return 0;
// }

#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>   // for signal(), SIGINT, SIGQUIT, SIGPIPE

int main(int ac, char **av)
{
    if (ac != 3) 
    {
        std::cout << "Usage: " << av[0] << " <port number> <password>" << std::endl;
        return 1;
    }
    int port = std::atoi(av[1]);
    std::string password = av[2];

    if (port < 1024 || port > 65535
        || (std::string(av[1]).find_first_not_of("0123456789") != std::string::npos) ||
        password.empty() || password.length() > 20)
    {
        std::cout << "Invalid Port number / Password!" << std::endl;
        return 1;
    }
    Server server(port, password);
    try {

        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        signal(SIGPIPE, SIG_IGN);
        server.start();
    }
    catch (const std::exception& e) 
    {
        server.close_fds();
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
