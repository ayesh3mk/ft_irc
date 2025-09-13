#include "Client.hpp"


Client::Client()
    : _fd(-1),
      _nick("*"),
      _username(""),
      _hostname("*"),
      _ip("0.0.0.0"),
      registered(false),
      logedin(false),
      buffer_in(""),
      buffer_out("") {}

Client::Client(int fd)
    : _fd(fd),
      _nick("*"),
      _username(""),
      _hostname("*"),
      _ip("0.0.0.0"),
      registered(false),
      logedin(false),
      buffer_in(""),
      buffer_out("") {}

Client::Client(int fd, const std::string& nick)
    : _fd(fd),
      _nick(nick),
      _username(""),
      _hostname("*"),
      _ip("0.0.0.0"),
      registered(false),
      logedin(false),
      buffer_in(""),
      buffer_out("") {}


int         Client::GetFd() const            { return _fd; }
std::string Client::GetNickName() const      { return _nick; }
std::string Client::GetUserName() const      { return _username; }
std::string Client::getHostname() const      { return _hostname; }
std::string Client::getIpAdd() const         { return _ip; }
std::string Client::getBufferIn() const      { return buffer_in; }
std::string Client::getBufferOut() const     { return buffer_out; }
std::string& Client::getBufferInRef()        { return buffer_in; }
std::string& Client::getBufferOutRef()       { return buffer_out; }
bool        Client::isRegistered() const     { return registered; }
bool        Client::GetLogedIn()             { return logedin; }
bool        Client::getRegistered()          { return registered; } 


void Client::SetFd(int fd)                   { _fd = fd; }
void Client::SetNickname(const std::string& n) { _nick = n; }
void Client::SetUsername(std::string& username) { _username = username; }
void Client::setHostname(const std::string& h)  { _hostname = h; }
void Client::setIpAdd(const std::string& ip)    { _ip = ip; }
void Client::setRegistered(bool status)         { registered = status; }
void Client::setLogedin(bool value)             { logedin = value; }

void Client::AddChannelInvite(const std::string& chNameNoHash) {
    _invites.insert(chNameNoHash);
}

void Client::RmChannelInvite(std::string& chNameNoHash) {
    _invites.erase(chNameNoHash);
}

bool Client::GetInviteChannel(std::string& chNameNoHash) {
    return _invites.find(chNameNoHash) != _invites.end();
}