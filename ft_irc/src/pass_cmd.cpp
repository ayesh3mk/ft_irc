#include "replies.hpp"
#include "Server.hpp"
#include "Client.hpp"
// bool Client::getRegistered(){return registered;}
int Server::GetFd(){return this->_port;}
// Client *Server::GetClient(int fd)
// {
//     for (size_t i = 0; i < this->_clients.size(); i++){
//         if(this->_clients[i].GetFd() == fd)
//             return &this->_clients[i];
//     }
//     return NULL;
// }
void Server::responsesender(std::string msg, int fd)
{
    if(send(fd, msg.c_str(), msg.size(), 0 ) == -1)
        std::cerr << "Response sending failed" << std::endl;
}
void Server::pass_cmd(int fd, std::string cmd)
{
    Client *cli = GetClient(fd);
    cmd = cmd.substr(4);
    size_t pos = cmd.find_first_not_of(" \t\v");
    if(pos<cmd.size())
    {
        cmd = cmd.substr(pos);
        if(cmd[0] == ':')
            cmd.erase(cmd.begin());
    }
    if ((pos == std::string::npos) || cmd.empty())
    {
        responsesender(ERR_NOTENOUGHPARAMS(std::string("*")), fd);
    }
    else if(!cli->getRegistered())
    {
        std::string pass = cmd;
        if(_password == pass)
            cli->setRegistered(true);
        else
        responsesender(ERR_PASSWDMISMATCH(std::string ("*")), fd);
    }
    else
        responsesender(ERR_ALREADYREGISTRED(cli->GetNickName()), fd);
}