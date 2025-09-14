#include "Channel.hpp"
#include <iostream>

Channel::Channel()
{
	this->invit_only = 0;
	this->topic = 0;
	this->limit = 0;
	this->topic_restriction = false;
	this->name = "";
	this->topic_name = "";
	char charaters[5] = {'i', 't', 'k', 'o', 'l'};
	for(int i = 0; i < 5; i++)
		modes.push_back(std::make_pair(charaters[i],false));
	this->created_at = "";
}

Channel::~Channel() {}
Channel::Channel(const Channel& src) 
{
    *this = src;
}

Channel &Channel::operator=(Channel const &src){
	if (this != &src){
		this->invit_only = src.invit_only;
		this->topic = src.topic;
		this->limit = src.limit;
		this->topic_restriction = src.topic_restriction;
		this->name = src.name;
		this->password = src.password;
		this->created_at = src.created_at;
		this->topic_name = src.topic_name;
		this->clients = src.clients;
		this->admins = src.admins;
		this->modes = src.modes;
        time_creation     = src.time_creation;
	}
	return *this;
}

bool Channel::hasMode(char modeChar) const {
    for (size_t i = 0; i < modes.size(); ++i)
        if (modes[i].first == modeChar && modes[i].second)
            return true;
    return false;
}

bool Channel::isOperator(const std::string& nickname) const {
    for (size_t i = 0; i < admins.size(); ++i)
        if (admins[i].nick == nickname)
            return true;
    return false;
}

bool Channel::isInviteOnly() const {
    return invite_only == 1;
}
bool Channel::clientInChannel(const std::string& nick)
{
    for (size_t i = 0; i < clients.size(); ++i)
        if (clients[i].nick == nick) return true;
    for (size_t i = 0; i < admins.size(); ++i)
        if (admins[i].nick == nick) return true;
    return false;
}


void Channel::SetInvitOnly(int invit_only)
{
    this->invit_only = invit_only;
}

void Channel::SetTopic(int topic)
{
    this->topic = topic;
}

void Channel::SetTime(std::string time)
{
    this->time_creation = time;
}

void Channel::SetLimit(int limit)
{
    this->limit = limit;
}

void Channel::SetTopicName(std::string topic_name)
{
    this->topic_name = topic_name;
}

void Channel::SetPassword(std::string password)
{
    this->password = password;
}

void Channel::SetName(std::string name)
{
    this->name = name;
}

void Channel::set_topicRestriction(bool value)
{
    this->topic_restriction = value;
}

void Channel::setModeAtindex(size_t index, bool mode)
{
    modes[index].second = mode;
    if (index == 0) // 0 = 'i' (invite-only)
        invite_only = mode ? 1 : 0;
}

void Channel::set_createiontime()
{
    std::ostringstream oss;
    oss << std::time(NULL);
    this->created_at = oss.str();
}

void Channel::setTopicMetaByNow(const std::string& setterNick)
{
    std::ostringstream oss;
    oss << std::time(NULL);   // epoch
    topic_set_by = setterNick;
    topic_set_at = oss.str();
}


int Channel::GetInvitOnly()
{
    return this->invit_only;
}

int Channel::GetTopic()
{
    return this->topic;
}


int Channel::GetLimit()
{
    return this->limit;
}

int Channel::GetClientsNumber()
{
    return this->clients.size() + this->admins.size();
}

bool Channel::Gettopic_restriction() const
{
    return this->topic_restriction;
}

bool Channel::getModeAtindex(size_t index)
{
    if (index < modes.size())
        return modes[index].second;
    return false;
}


std::string Channel::GetTopicName()
{
    return this->topic_name;
}

std::string Channel::GetPassword()
{
    return this->password;
}

std::string Channel::GetName()
{
    return this->name;
}

std::string Channel::GetTime()
{
    return this->time_creation;
}

std::string Channel::get_creationtime()
{
    return this->created_at;
}

std::string Channel::getModes()
{
    std::string mode;
    for (size_t i = 0; i < modes.size(); i++)
    {
        if (modes[i].first != 'o' && modes[i].second)
            mode.push_back(modes[i].first);
    }
    if (!mode.empty())
        mode.insert(mode.begin(), '+');
    return mode;
}

const std::string& Channel::getTopicSetBy() const
{
    return topic_set_by;
}

const std::string& Channel::getTopicSetAt() const
{
    return topic_set_at;
}


std::string Channel::clientChannel_list()
{
    std::string list;
    for (size_t i = 0; i < admins.size(); ++i) {
        list += "@" + admins[i].nick;
        if ((i + 1) < admins.size()) list += " ";
    }
    if (!clients.empty() && !admins.empty()) list += " ";
    for (size_t i = 0; i < clients.size(); ++i) {
        list += clients[i].nick;
        if ((i + 1) < clients.size()) list += " ";
    }
    return list;
}

ChanMember* Channel::get_client(int fd)
{
    for (size_t i = 0; i < clients.size(); ++i)
        if (clients[i].fd == fd) return &clients[i];
    return NULL;
}

ChanMember* Channel::get_admin(int fd)
{
    for (size_t i = 0; i < admins.size(); ++i)
        if (admins[i].fd == fd) return &admins[i];
    return NULL;
}

ChanMember* Channel::GetClientInChannel(std::string name)
{
    for (size_t i = 0; i < clients.size(); ++i)
        if (clients[i].nick == name) return &clients[i];
    for (size_t i = 0; i < admins.size(); ++i)
        if (admins[i].nick == name) return &admins[i];
    return NULL;
}

//methods

void Channel::add_client(const ChanMember& m) { clients.push_back(m); }
void Channel::add_admin(const ChanMember& m)  { admins.push_back(m); }

void Channel::remove_client(int fd)
{
    for (std::vector<ChanMember>::iterator it = clients.begin(); it != clients.end(); ++it)
        if (it->fd == fd) { clients.erase(it); break; }
}

void Channel::remove_admin(int fd)
{
    for (std::vector<ChanMember>::iterator it = admins.begin(); it != admins.end(); ++it)
        if (it->fd == fd) { admins.erase(it); break; }
}

bool Channel::change_clientToAdmin(std::string& nick)
{
    for (size_t i = 0; i < clients.size(); ++i)
        if (clients[i].nick == nick) {
            admins.push_back(clients[i]);
            clients.erase(clients.begin() + i);
            return true;
        }
    return false;
}

bool Channel::change_adminToClient(std::string& nick)
{
    for (size_t i = 0; i < admins.size(); ++i)
        if (admins[i].nick == nick) {
            clients.push_back(admins[i]);
            admins.erase(admins.begin() + i);
            return true;
        }
    return false;
}

void Channel::sendTo_all(std::string rpl1)
{
    for (size_t i = 0; i < admins.size(); ++i)
        if (send(admins[i].fd, rpl1.c_str(), rpl1.size(), 0) == -1)
            std::cerr << "send() failed to admin fd " << admins[i].fd << std::endl;

    for (size_t i = 0; i < clients.size(); ++i)
        if (send(clients[i].fd, rpl1.c_str(), rpl1.size(), 0) == -1)
            std::cerr << "send() failed to client fd " << clients[i].fd << std::endl;
}

void Channel::sendTo_all(std::string rpl1, int exceptFd)
{
    for (size_t i = 0; i < admins.size(); ++i)
        if (admins[i].fd != exceptFd)
            if (send(admins[i].fd, rpl1.c_str(), rpl1.size(), 0) == -1)
                std::cerr << "send() failed to admin fd " << admins[i].fd << std::endl;

    for (size_t i = 0; i < clients.size(); ++i)
        if (clients[i].fd != exceptFd)
            if (send(clients[i].fd, rpl1.c_str(), rpl1.size(), 0) == -1)
                std::cerr << "send() failed to client fd " << clients[i].fd << std::endl;
}
