#include "Server.hpp"
#include "Client.hpp"
#include "replies.hpp"

std::vector<std::string> Server::splitter(std::string& cmd)
{
    std::vector<std::string> vec;
    std::istringstream stm(cmd);
    std::string token;

    while(stm >> token)
    {
        vec.push_back(token);
        token.clear();
    }
    return vec;
}
bool Server::notregistered(int fd)
{
	Client* cli = GetClient(fd);

if (/* no need to check !cli since it's an object */
    cli->GetNickName().empty() ||
    cli->GetUserName().empty() ||
    cli->GetNickName() == "*" ||
    !cli->GetLogedIn())
    {
    		return false;
    }
    return true;
}

void Server::parse(std::string cmd, int fd)
{
    if(cmd.empty())
        return;
    std::vector<std::string> split_cmd = splitter(cmd);
    size_t found = cmd.find_first_not_of(" \t\v");
    if(found != std::string::npos)
        cmd = cmd.substr(found);
    if(split_cmd.size() && ((split_cmd[0] == "PASS") || (split_cmd[0] == "pass")))
        pass_cmd( fd, cmd);
    else if(cmd.size() && ((split_cmd[0] == "NICK") || (split_cmd[0] == "nick")))
        nick_cmd(cmd, fd);
    else if(split_cmd.size() && ((split_cmd[0] == "USER") || (split_cmd[0] == "user")))
        user_cmd(cmd, fd);
    // else if(split_cmd.size() && (split_cmd[0] == "QUIT") || (split_cmd[0] == "join"))
    //     QUIT(split_cmd, fd);
    else if(notregistered(fd))
    {
        if(split_cmd.size() && ((split_cmd[0] == "JOIN") || (split_cmd[0] == "join")))
            JOIN(cmd, fd);
        else if(split_cmd.size() && ((split_cmd[0] == "INVITE") || (split_cmd[0] == "invite")))
            INVITE(cmd, fd);
        else if(split_cmd.size() && ((split_cmd[0] == "MODE") || (split_cmd[0] == "mode")))
            MODE(cmd, fd);
        else if(split_cmd.size() && ((split_cmd[0] == "KICK") || (split_cmd[0] == "kick")))\
            KICK(cmd, fd);
        else if(split_cmd.size() && ((split_cmd[0] == "PRIVMSG") || (split_cmd[0] == "privmsg")))
            PRIVMSG(cmd, fd);
        else if(split_cmd.size() && ((split_cmd[0] == "TOPIC") || (split_cmd[0] == "topic")))
            TOPIC(cmd, fd);      
    }
    else if (!notregistered(fd))
        responsesender(ERR_NOTREGISTERED(std::string("*")),fd);
}