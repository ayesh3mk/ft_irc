#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include <string>
#include <map>
#include <vector>
#include <netinet/in.h>
#include <iostream>
#include <sstream> 
#include <utility>  
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <set>
#include <map>
#include <utility>  

class Client;
class Channel;


class Server
{
private:
    int _port;
    std::string _password;
    static bool Signal;   
    int _server_fd;
    struct sockaddr_in _address;

    std::map<int, Client> _clients;
    std::vector<struct pollfd> _pollfds;
    //aysha
    // std::map<int, Client>              _clients;   // fd -> Client
    std::map<std::string, Channel*>    _channels;  // key = channel WITHOUT '#'
    // std::vector<struct pollfd> _pollfds;

    void setupServerSocket();
    void makeNonBlocking(int fd);
    void handleNewConnection();
    void handleClientInput(int fd);
    void disconnectClient(int fd);
    void errorExit(const std::string &msg);

    //aysha
    Client*  GetClient(int fd);
    Channel* getChannel(const std::string& nameNoHash);
    Channel* ensureChannel(const std::string& nameNoHash);
    int      SearchForClients(const std::string& nickname) const;

    int  SplitJoin(std::vector<std::pair<std::string,std::string> >& out, std::string cmd, int fd);
    void ExistCh(std::vector<std::pair<std::string,std::string> >& token, int i, Channel* ch, int fd);
    void NotExistCh(std::vector<std::pair<std::string,std::string> >& token, int i, int fd);
    void sendResponse(int fd, const std::string& msg);  // wrapper
    Client* findClientByNick(const std::string& nick);   

public:
    Server(int port, const std::string &password);
    std::vector<std::string> splitter(std::string& cmd);
    static void SignalHandler(int signum);  
    ~Server();

    void start();
    void close_fds();
    void eventLoop();
    //insha
    int  GetFd();
    void parse(std::string cmd, int fd);
    bool nickNameInUse(std::string& nickname);
    void pass_cmd(int fd, std::string pass);
    void nick_cmd(std::string cmd, int fd);
    void user_cmd(std::string& cmd, int fd);
    bool notregistered(int fd);
    // Client* GetClient(int fd);
    bool is_validNickname(std::string& nickname);
    void responsesender(std::string msg, int fd);

    //aysha
    void JOIN(std::string cmd, int fd);
    void INVITE(std::string cmd, int fd);
    void MODE(std::string cmd, int fd);
    void KICK(std::string cmd, int fd);
    void PRIVMSG(std::string cmd, int fd);
    void TOPIC(std::string cmd, int fd);
};

#endif 