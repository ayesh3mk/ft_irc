#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <utility>
#include <sys/socket.h>
#include <iostream>
#include <ctime>
#include <sstream>
#include <map>  
#include <set>

struct ChanMember {
    int         fd;
    std::string nick;
};

class Channel
{
private:
    int         invit_only;
    int         topic;
    int         limit;
    bool        topic_restriction;


    std::string _modes;             
    int invite_only;   

    std::string name;
    std::string time_creation;
    std::string password;
    std::string created_at;
    std::string topic_name;
    std::string topic_set_by;       
    std::string topic_set_at;         
    

    std::vector<ChanMember> clients;  
    std::vector<ChanMember> admins;   
    std::vector< std::pair<char,bool> > modes; 

public:
    Channel();
    ~Channel();
    Channel(const Channel& src);
    Channel& operator=(const Channel& src);

    bool hasMode(char modeChar) const;
    bool isOperator(const std::string& nickname) const;
    bool isInviteOnly() const;
    bool clientInChannel(const std::string& nick);

    void SetInvitOnly(int invit_only);
    void SetTopic(int topic);
    void SetTime(std::string time);
    void SetLimit(int limit);
    void SetTopicName(std::string topic_name);
    void SetPassword(std::string password);
    void SetName(std::string name);
    void set_topicRestriction(bool value);
    void setModeAtindex(size_t index, bool mode);
    void set_createiontime();
    void setTopicMetaByNow(const std::string& setterNick);

    const std::string& getTopicSetBy() const;
    const std::string& getTopicSetAt() const;


    int         GetInvitOnly();
    int         GetTopic();
    int         GetLimit();
    int         GetClientsNumber();
    bool        Gettopic_restriction() const;
    bool        getModeAtindex(size_t index);

    std::string GetTopicName();
    std::string GetPassword();
    std::string GetName();
    std::string GetTime();
    std::string get_creationtime();
    std::string getModes();
    std::string clientChannel_list();

    ChanMember* get_client(int fd);
    ChanMember* get_admin(int fd);
    ChanMember* GetClientInChannel(std::string name);


    void add_client(const ChanMember& m);
    void add_admin(const ChanMember& m);
    void remove_client(int fd);
    void remove_admin(int fd);
    bool change_clientToAdmin(std::string& nick);
    bool change_adminToClient(std::string& nick);

    void sendTo_all(std::string rpl1);
    void sendTo_all(std::string rpl1, int exceptFd);
};

#endif