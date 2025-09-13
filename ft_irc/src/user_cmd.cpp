#include "Server.hpp"
#include "Client.hpp"
#include "replies.hpp"


// std::string Client::GetUserName()const{return this->_username;}
// void Client::SetUsername(std::string& username){this->_username = username;}
// void Client::setLogedin(bool value){this->logedin = value;}
void	Server::user_cmd(std::string& cmd, int fd)
{
	std::vector<std::string> splited_cmd = splitter(cmd);

	Client *cli = GetClient(fd); 
	if((cli && splited_cmd.size() < 5))
		{responsesender(ERR_NOTENOUGHPARAMS(cli->GetNickName()), fd); return; }
	if(!cli  || !cli->getRegistered())
		responsesender(ERR_NOTREGISTERED(std::string("*")), fd);
	else if (cli && !cli->GetUserName().empty())
		{responsesender(ERR_ALREADYREGISTRED(cli->GetNickName()), fd); return;}
	else
		cli->SetUsername(splited_cmd[1]);
	if(cli && cli->getRegistered() && !cli->GetUserName().empty() && !cli->GetNickName().empty() && cli->GetNickName() != "*"  && !cli->GetLogedIn())
	{
		cli->setLogedin(true);
		responsesender(RPL_CONNECTED(cli->GetNickName()), fd);
	}
}