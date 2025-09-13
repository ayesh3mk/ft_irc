#include "Server.hpp"
#include "Client.hpp"

Server::Server(int port, const std::string &password)
    : _port(port), _password(password), _server_fd(-1) {}

Server::~Server() 
{
    close_fds();
    _pollfds.clear();
    _clients.clear();
}

void Server::start() 
{
    setupServerSocket();
    std::cout << "*****Server is running***"<< std::endl;
    eventLoop();
}

void Server::setupServerSocket() 
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0) errorExit("socket() failed");

    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        errorExit("setsockopt() failed");  //eth fail aayal bind() failed: Address already in use enna error kanicku

    makeNonBlocking(_server_fd);

    std::memset(&_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_port);

    if (bind(_server_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0)
        errorExit("bind() failed");

    if (listen(_server_fd, 10) < 0)
        errorExit("listen() failed");

    struct pollfd pfd;
    pfd.fd = _server_fd;
    pfd.events = POLLIN;
    _pollfds.push_back(pfd);
}

void Server::makeNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) flags = 0; 
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        errorExit("fcntl() failed");
}

void Server::eventLoop()
{
    while (!Signal) 
    {  // stop when signal is received
        int ret = poll(&_pollfds[0], _pollfds.size(), -1);
        if (ret < 0) continue;

        for (size_t i = 0; i < _pollfds.size(); i++) {
            if (_pollfds[i].revents & POLLIN) {
                if (_pollfds[i].fd == _server_fd) {
                    handleNewConnection();
                } else {
                    handleClientInput(_pollfds[i].fd);
                }
            }

            if (_pollfds[i].revents & POLLOUT) {
                sendClientData(_pollfds[i].fd);
            }
        }
    }
    close_fds();
    std::cout << "The Server Closed!" << std::endl;
}



void Server::handleNewConnection() {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    int client_fd = accept(_server_fd, (struct sockaddr *)&client_addr, &addrlen);
    if (client_fd < 0) return;

    makeNonBlocking(client_fd);

    struct pollfd pfd;
    pfd.fd = client_fd;
    pfd.events = POLLIN;
    _pollfds.push_back(pfd);

    _clients.insert(std::make_pair(client_fd, Client(client_fd)));

    std::cout << "[Server] Client " << client_fd << " is connected." << std::endl;
}
bool Server::Signal = false;  // definition

void Server::SignalHandler(int signum) {
    (void)signum;
    std::cout << "    " << std::endl;
    Server::Signal = true;
}

void Server::close_fds() {
    // Disconnect all clients
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        std::cout << "Client " << it->first << " Disconnected" << std::endl;
        close(it->first);
    }

    // Disconnect server socket
    if (_server_fd != -1) {
        std::cout << "Server " << _server_fd << " Disconnected" << std::endl;
        close(_server_fd);
        _server_fd = -1;
    }
}

void Server::disconnectClient(int fd) 
{
    std::cout << "[Server] Client " << fd << " is disconnected." << std::endl;
    close(fd);
    _clients.erase(fd);

    for (size_t i = 0; i < _pollfds.size(); i++) {
        if (_pollfds[i].fd == fd) {
            _pollfds.erase(_pollfds.begin() + i);
            break;
        }
    }
}

void Server::handleClientInput(int fd) {
    char buffer[512];
    int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        disconnectClient(fd);
        return;
    }

    buffer[bytes] = '\0';
    Client &client = _clients[fd];
    std::string &buf = client.getBufferInRef();
    buf += std::string(buffer);

    size_t pos;
    while ((pos = buf.find("\r\n")) != std::string::npos ||
           (pos = buf.find("\n")) != std::string::npos) {
        std::string msg = buf.substr(0, pos);
        buf.erase(0, pos + ((buf[pos] == '\r') ? 2 : 1));
        parse(msg, fd);  //insha 
    }
}

void Server::sendClientData(int fd) 
{
    Client &client = _clients[fd];
    std::string &buf = client.getBufferOutRef();
    if (buf.empty()) return;

    int bytes = send(fd, buf.c_str(), buf.size(), 0);
    if (bytes > 0) {
        buf.erase(0, bytes);
    } else if (bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        disconnectClient(fd);
    }
}

void Server::sendToClient(int fd, const std::string &msg) 
{
    Client &client = _clients[fd];
    std::string &buf = client.getBufferOutRef();
    buf += msg + "\r\n";
}

void Server::errorExit(const std::string &msg)
{
    perror(msg.c_str());
    exit(EXIT_FAILURE);
}
