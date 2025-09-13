#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>

class Client {
private:

    int         _fd;
    std::string _nick;
    std::string _username;
    std::string _hostname;
    std::string _ip;
    bool        registered;
    bool        logedin;

    std::string buffer_in;
    std::string buffer_out;

    std::set<std::string> _invites;

public:
    Client();
    Client(int fd);
    Client(int fd, const std::string& nick);

    int         GetFd() const;
    std::string GetNickName() const;
    std::string GetUserName() const;
    std::string getHostname() const;
    std::string getIpAdd() const;
    std::string getBufferIn() const;
    std::string getBufferOut() const;
    std::string& getBufferInRef();
    std::string& getBufferOutRef();
    bool        isRegistered() const;
    bool        GetLogedIn(); 
    bool        getRegistered(); 

    // --- Setters ---
    void SetFd(int fd);
    void SetNickname(const std::string& n);
    void SetUsername(std::string& username);   // kept signature as-is to match your use
    void setHostname(const std::string& h);
    void setIpAdd(const std::string& ip);
    void setRegistered(bool status);
    void setLogedin(bool value);

    // --- Invite Helpers ---
    void AddChannelInvite(const std::string& chNameNoHash);
    void RmChannelInvite(std::string& chNameNoHash);
    bool GetInviteChannel(std::string& chNameNoHash);
};

#endif // CLIENT_HPP
