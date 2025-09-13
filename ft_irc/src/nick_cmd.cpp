#include "Server.hpp"
#include "replies.hpp"
// bool Client::GetLogedIn(){return this->logedin;}
bool Server::nickNameInUse(std::string& nickname)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i].GetNickName() == nickname)
			return true;
	}
	return false;
}

// std::string Client::GetNickName() const { return this->_nick; }
bool Server::is_validNickname(std::string& nickname)
{
		
	if(!nickname.empty() && (nickname[0] == '&' || nickname[0] == '#' || nickname[0] == ':'))
		return false;
	for(size_t i = 1; i < nickname.size(); i++)
	{
		if(!std::isalnum(nickname[i]) && nickname[i] != '_')
			return false;
	}
	return true;
}
// void Client::SetNickname(const std::string& nickName) {
//     _nick = nickName;
// }
void Server::nick_cmd(std::string cmd, int fd)
{
    std::string inuse;
    cmd = cmd.substr(4);
    size_t pos = cmd.find_first_not_of("\t\v ");
    if(pos < cmd.size())
    {
        cmd = cmd.substr(pos);
        if(cmd[0] == ':')
            cmd.erase(cmd.begin());
    }
    Client *cli = GetClient(fd);
    if(pos ==std::string::npos || cmd.empty())
        {responsesender(ERR_NOTENOUGHPARAMS(std::string("*")),fd); return;}
    if(nickNameInUse(cmd) && cli->GetNickName() != cmd){
        inuse = "*";
        if(cli->GetNickName().empty())
            cli->SetNickname(inuse);
        responsesender(ERR_NICKINUSE(std::string(cmd)), fd);
        return;
    }
    if(!is_validNickname(cmd))
    {
        responsesender(ERR_ERRONEUSNICK(std::string (cmd)), fd);
        return;
    }
    else{
        if(cli && cli->getRegistered())
        {
            std::string oldnick = cli->GetNickName();
            cli->SetNickname(cmd);
            // for(size_t i = 0; i < channels.size(); i++){
            //     Client *cl = channels[i].GetClientInChannel(oldnick); //from ayesha
            //     if(cl)
            //         cl->SetNickname(cmd);
            // }
            if(!oldnick.empty() && oldnick != cmd)
            {
                if(oldnick == "*" && !cli->GetUserName().empty())
                {
                    cli->setLogedin(true);
                    responsesender(RPL_CONNECTED(cli->GetNickName()), fd);
                    responsesender(RPL_NICKCHANGE(cli->GetNickName(), cmd), fd);
                    
                }
                else
                    responsesender(RPL_NICKCHANGE(oldnick, cmd), fd);
                return;
            }
        }
        else if(cli && !cli->getRegistered())
            responsesender(ERR_NOTREGISTERED(cmd), fd);
    }
    	if(cli && cli->getRegistered() && !cli->GetUserName().empty() && !cli->GetNickName().empty() && cli->GetNickName() != "*" && !cli->GetLogedIn())
	    {
	    	cli->setLogedin(true);
	    	responsesender(RPL_CONNECTED(cli->GetNickName()), fd);
	    }
    
}