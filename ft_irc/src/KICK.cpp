#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "replies.hpp"
#include <sstream>



void Server::KICK(std::string cmd, int fd)
{
    Client* chi = GetClient(fd);
    if (!chi) return;

    std::istringstream iss(cmd);
    std::string verb, chanTok, targetNick, comment;
    iss >> verb >> chanTok >> targetNick;
    std::getline(iss, comment);
    if (!comment.empty() && comment[0] == ':')
        comment.erase(0, 1);
    if (comment.empty()) comment = chi->GetNickName();

    const std::string nick = chi->GetNickName();

    if (chanTok.empty() || targetNick.empty()) {
        sendResponse(fd, ERR_NEEDMOREPARAMS(nick, "KICK"));
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

    if (!ch->get_admin(fd)) {
        sendResponse(fd, ERR_CHANOPRIVSNEEDED(nick, ch->GetName()));
        return;
    }

    ChanMember* tgt = ch->GetClientInChannel(targetNick);
    if (!tgt) {
        sendResponse(fd, ERR_USERNOTINCHANNEL(nick, targetNick, ch->GetName()));
        return;
    }

    ch->remove_client(tgt->fd);
    ch->remove_admin(tgt->fd);

    std::string line = RPL_KICK(nick, ch->GetName(), targetNick, comment);
    ch->sendTo_all(line);
}