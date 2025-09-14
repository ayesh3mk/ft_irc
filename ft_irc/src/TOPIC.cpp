#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "replies.hpp"
#include <sstream>
#include <string>

static inline void ltrim(std::string &s) {
    size_t p = s.find_first_not_of(" \t\r\n");
    if (p == std::string::npos) s.clear();
    else s.erase(0, p);
}

void Server::TOPIC(std::string cmd, int fd)
{
    Client* who = GetClient(fd);
    if (!who) return;

    std::istringstream iss(cmd);
    std::string verb, chanTok;
    iss >> verb >> chanTok;

    std::string rest;
    std::getline(iss, rest);
    ltrim(rest);
    if (!rest.empty() && rest[0] == ':')
        rest.erase(0, 1);

    const std::string nick = who->GetNickName();

    if (chanTok.empty()) {
        sendResponse(fd, ERR_NEEDMOREPARAMS(nick, "TOPIC"));
        return;
    }
    if (chanTok[0] != '#') {
        sendResponse(fd, ERR_CHANNELNOTFOUND(nick, chanTok));
        return;
    }

    std::string chNoHash = chanTok.substr(1);
    Channel* ch = getChannel(chNoHash);
    if (!ch) {
        sendResponse(fd, ERR_CHANNELNOTFOUND(nick, chanTok));
        return;
    }

    {
        std::string tmp = nick;
        if (!ch->clientInChannel(tmp)) {
            sendResponse(fd, ERR_NOTONCHANNEL(nick, ch->GetName()));
            return;
        }
    }

    if (rest.empty()) {
        if (ch->GetTopicName().empty()) {
            sendResponse(fd, RPL_NOTOPIC(nick, ch->GetName()));
        } else {
            sendResponse(fd, RPL_TOPICIS(nick, ch->GetName(), ch->GetTopicName()));
            if (!ch->getTopicSetBy().empty() && !ch->getTopicSetAt().empty()) {
                sendResponse(fd, RPL_TOPICWHOTIME(nick, ch->GetName(),
                                                  ch->getTopicSetBy(),
                                                  ch->getTopicSetAt()));
            }
        }
        return;
    }

    if (ch->Gettopic_restriction()) {
        ChanMember* op = ch->get_admin(fd);
        if (!op) {
            sendResponse(fd, ERR_CHANOPRIVSNEEDED(nick, ch->GetName()));
            return;
        }
    }

    ch->SetTopicName(rest);
    ch->setTopicMetaByNow(nick);

    std::string line = ":" + who->getHostname() + "@" + who->getIpAdd()
                     + " TOPIC #" + ch->GetName() + " :" + rest + CRLF;
    ch->sendTo_all(line);
}