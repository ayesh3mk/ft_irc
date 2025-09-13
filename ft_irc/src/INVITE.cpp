#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "replies.hpp"
#include <sstream>
#include <string>

Client* Server::findClientByNick(const std::string& nick)
{
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second.GetNickName() == nick)
            return &it->second;
    }
    return NULL;
}

void Server::INVITE(std::string cmd, int fd)
{
    Client* chi = GetClient(fd);
    if (!chi) return;

    std::istringstream iss(cmd);
    std::string verb, targetNick, chanTok;
    iss >> verb >> targetNick >> chanTok;

    const std::string inviterNick = chi->GetNickName();

    // ERR_NEEDMOREPARAMS (461)
    if (targetNick.empty() || chanTok.empty()) {
        sendResponse(fd, ERR_NEEDMOREPARAMS(inviterNick, "INVITE"));
        return;
    }

    // Channel name must start with '#'
    if (chanTok[0] != '#') {
        sendResponse(fd, ERR_CHANNELNOTFOUND(inviterNick, chanTok));
        return;
    }

    std::string chNoHash = chanTok.substr(1);
    Channel* ch = getChannel(chNoHash);
    
    // ERR_NOSUCHCHANNEL (403)
    if (!ch) {
        sendResponse(fd, ERR_CHANNELNOTFOUND(inviterNick, chanTok));
        return;
    }

    // ERR_NOTONCHANNEL (442)
    if (!ch->clientInChannel(inviterNick)) {
        sendResponse(fd, ERR_NOTONCHANNEL(inviterNick, ch->GetName()));
        return;
    }

    // ✅ NEW: Permission check for +i mode
    if (ch->isInviteOnly() && !ch->isOperator(inviterNick)) {
        std::cout << "[DEBUG] Blocked INVITE: " << inviterNick << " is not an operator on invite-only channel.\n";
        sendResponse(fd, ERR_CHANOPRIVSNEEDED(inviterNick, ch->GetName()));
        return;
    }

    // ERR_NOSUCHNICK (401)
    Client* target = findClientByNick(targetNick);
    if (!target) {
        sendResponse(fd, ERR_NOSUCHNICK(inviterNick, targetNick));
        return;
    }

    // ERR_USERONCHANNEL (443)
    if (ch->clientInChannel(targetNick)) {
        sendResponse(fd, ERR_USERONCHANNEL(inviterNick, targetNick, ch->GetName()));
        return;
    }

    // Add channel to target's invite list
    target->AddChannelInvite(ch->GetName());

    // RPL_INVITING (341)
    sendResponse(fd, RPL_INVITING(inviterNick, targetNick, ch->GetName()));

    // Send invite to the target client
    sendResponse(target->GetFd(), RPL_INVITE(
        chi->getHostname(),
        chi->getIpAdd(),
        targetNick,
        ch->GetName()
    ));
}