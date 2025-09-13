	// •	461 need more params
	// •	407 too many channels (targets)
	// •	403 bad channel (no #)
	// •	405 already in too many channels
	// •	475 bad key (+k)
	// •	473 invite-only (+i)
	// •	471 channel full (+l)
	// •	(Optional) 443 already on channel
	// •	331/332/333 topic replies

#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "replies.hpp"

#include <sstream>  
#include <string>
#include <vector>
#include <map>


// ---------- tiny send wrapper ----------
void Server::sendResponse(int fd, const std::string& msg)
{
    // If you already have _sendResponse(fd, msg), call that instead.
    // This version writes directly on the socket.
    if (fd < 0 || msg.empty()) return;
    (void)send(fd, msg.c_str(), msg.size(), 0);
}


Client* Server::GetClient(int fd)
{
    std::map<int, Client>::iterator it = _clients.find(fd);
    return (it == _clients.end()) ? NULL : &it->second;
}

Channel* Server::getChannel(const std::string& nameNoHash)
{
    std::map<std::string, Channel*>::iterator it = _channels.find(nameNoHash);
    return (it == _channels.end()) ? NULL : it->second;
}

Channel* Server::ensureChannel(const std::string& nameNoHash)
{
    Channel* ch = getChannel(nameNoHash);
    if (ch)
        return ch;

    ch = new Channel();
    ch->SetName(nameNoHash);   // store WITHOUT '#'
    ch->set_createiontime();   // your helper
    _channels[nameNoHash] = ch;
    return ch;
}

int Server::SearchForClients(const std::string& nickname) const
{
    int count = 0;
    for (std::map<std::string, Channel*>::const_iterator it = _channels.begin();
         it != _channels.end(); ++it)
    {
        Channel* ch = it->second;
        if (ch && ch->GetClientInChannel(nickname)) ++count;
    }
    return count;
}

// ---------- parser: "JOIN #a,#b [k1,k2]" ----------
int Server::SplitJoin(std::vector<std::pair<std::string,std::string> >& out, std::string cmd, int fd)
{
    out.clear();

    std::vector<std::string> words;
    std::istringstream iss(cmd);
    std::string w;
    while (iss >> w) words.push_back(w);

    Client* c = GetClient(fd);
    const std::string nick = c ? c->GetNickName() : "*";

    if (words.size() < 2) {
        sendResponse(fd, ERR_NEEDMOREPARAMS(nick, "JOIN"));
        return 0;
    }

    words.erase(words.begin());                   // drop "JOIN"
    const std::string chStr  = words[0];         // "#a,#b"
    const std::string keyStr = (words.size() > 1) ? words[1] : ""; // "k1,k2"


    std::string buf;
    for (size_t i = 0; i < chStr.size(); ++i) {
        if (chStr[i] == ',') { out.push_back(std::make_pair(buf, "")); buf.clear(); }
        else                 { buf += chStr[i]; }
    }
    out.push_back(std::make_pair(buf, "")); // last

    if (!keyStr.empty()) {
        size_t j = 0; buf.clear();
        for (size_t i = 0; i < keyStr.size(); ++i) {
            if (keyStr[i] == ',') {
                if (j < out.size()) out[j].second = buf;
                ++j; buf.clear();
            } else buf += keyStr[i];
        }
        if (!buf.empty() && j < out.size()) out[j].second = buf;
    }

    for (size_t i = 0; i < out.size(); ++i) {
        if (out[i].first.empty()) { out.erase(out.begin() + i--); continue; }
        if (out[i].first[0] != '#') {
            sendResponse(fd, ERR_CHANNELNOTFOUND(nick, out[i].first));
            out.erase(out.begin() + i--);
        } else {
            out[i].first.erase(out[i].first.begin()); // drop '#'
            if (out[i].first.empty()) { out.erase(out.begin() + i--); }
        }
    }
    return 1;
}


void Server::ExistCh(std::vector<std::pair<std::string,std::string> >& token,
                     int i, Channel* ch, int fd)
{
    Client* cli = GetClient(fd);
    if (!cli || !ch) return;

    const std::string nick = cli->GetNickName();

    if (ch->GetClientInChannel(nick))
        return;

    if (SearchForClients(nick) >= 10) {
        sendResponse(fd, ERR_TOOMANYCHANNELS(nick, ch->GetName()));
        return;
    }

    const std::string& joinKey = token[i].second;
    if (!ch->GetPassword().empty() && ch->GetPassword() != joinKey) {
        std::string chNoHash = token[i].first;
        if (!cli->GetInviteChannel(chNoHash)) {
            sendResponse(fd, ERR_BADCHANNELKEY(nick, ch->GetName()));
            return;
        }
        // invited -> bypass bad key
    }

    if (ch->GetInvitOnly()) {
        std::string chNoHash = token[i].first;
        if (!cli->GetInviteChannel(chNoHash)) {
            sendResponse(fd, ERR_INVITEONLYCHAN(nick, ch->GetName()));
            return;
        }
        cli->RmChannelInvite(chNoHash);
    }

    if (ch->GetLimit() && ch->GetClientsNumber() >= ch->GetLimit()) {
        sendResponse(fd, ERR_CHANNELISFULL(nick, ch->GetName()));
        return;
    }

    ChanMember m; m.fd = cli->GetFd(); m.nick = nick;
    ch->add_client(m);

    sendResponse(fd, RPL_JOINMSG(cli->getHostname(), cli->getIpAdd(), ch->GetName()));
  
    if (!ch->GetTopicName().empty()) {
        sendResponse(fd, RPL_TOPICIS(nick, ch->GetName(), ch->GetTopicName()));

        if (!ch->getTopicSetBy().empty() && !ch->getTopicSetAt().empty()) {
            sendResponse(fd, RPL_TOPICWHOTIME(nick, ch->GetName(), ch->getTopicSetBy(), ch->getTopicSetAt()));
        }
    } else {
        sendResponse(fd, RPL_NOTOPIC(nick, ch->GetName()));
    }

    sendResponse(fd, RPL_NAMREPLY(nick, ch->GetName(), ch->clientChannel_list()));
    sendResponse(fd, RPL_ENDOFNAMES(nick, ch->GetName()));
    sendResponse(fd, RPL_CREATIONTIME(nick, ch->GetName(), ch->get_creationtime()));
    ch->sendTo_all(RPL_JOINMSG(cli->getHostname(), cli->getIpAdd(), ch->GetName()), fd);
}


void Server::NotExistCh(std::vector<std::pair<std::string,std::string> >& token,
                        int i, int fd)
{
    Client* cli = GetClient(fd);
    if (!cli) return;

    if (SearchForClients(cli->GetNickName()) >= 10) {
        sendResponse(fd, ERR_TOOMANYCHANNELS(cli->GetNickName(), token[i].first));
        return;
    }

    Channel* ch = ensureChannel(token[i].first);

    // First joiner = operator
    ChanMember op; op.fd = cli->GetFd(); op.nick = cli->GetNickName();
    ch->add_admin(op);

    sendResponse(fd, RPL_JOINMSG(cli->getHostname(), cli->getIpAdd(), ch->GetName()));
    sendResponse(fd, RPL_NOTOPIC(cli->GetNickName(), ch->GetName()));
    sendResponse(fd, RPL_NAMREPLY(cli->GetNickName(), ch->GetName(), ch->clientChannel_list()));
    sendResponse(fd, RPL_ENDOFNAMES(cli->GetNickName(), ch->GetName()));
    sendResponse(fd, RPL_CREATIONTIME(cli->GetNickName(), ch->GetName(), ch->get_creationtime()));
}

void Server::JOIN(std::string cmd, int fd)
{
    Client* cli = GetClient(fd);
    if (!cli)
    {
        sendResponse(fd, ERR_NOTREGISTERED(std::string("*")));
        return;
    }

    std::vector<std::pair<std::string,std::string> > token;
    if (!SplitJoin(token, cmd, fd)) return;

    if (token.size() > 10) {
        sendResponse(fd, ERR_TOOMANYTARGETS(cli->GetNickName(), "JOIN"));
        return;
    }

    for (size_t i = 0; i < token.size(); ++i) {
        Channel* ch = getChannel(token[i].first);
        if (ch)  ExistCh(token, static_cast<int>(i), ch, fd);
        else     NotExistCh(token, static_cast<int>(i), fd);
    }
}