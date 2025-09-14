#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "replies.hpp"
#include <sstream>

void Server::PRIVMSG(std::string cmd, int fd)
{
    Client* sender = GetClient(fd);
    if (!sender) return;

    std::istringstream iss(cmd);
    std::string verb, target, msg;
    iss >> verb >> target;
    std::getline(iss, msg);

    if (target.empty()) {
        sendResponse(fd, ERR_NORECIPIENT(sender->GetNickName(), "PRIVMSG"));
        return;
    }
    if (!msg.empty() && msg[0] == ' ')
        msg.erase(0, 1);
    if (!msg.empty() && msg[0] == ':')
        msg.erase(0, 1);
    if (msg.empty()) {
        sendResponse(fd, ERR_NOTEXTTOSEND(sender->GetNickName()));
        return;
    }
    std::string line = ":" + sender->GetNickName() + "!" + "@" + sender->getIpAdd() + " PRIVMSG " + target + " :" + msg + CRLF;

    if (target[0] == '#') {
        std::string chNoHash = target.substr(1);
        Channel* ch = getChannel(chNoHash);
        if (!ch) {
            sendResponse(fd, ERR_CANNOTSENDTOCHAN(sender->GetNickName(), chNoHash));
            return;
        }

        std::string tmp = sender->GetNickName();
        if (!ch->clientInChannel(tmp)) {
            sendResponse(fd, ERR_CANNOTSENDTOCHAN(sender->GetNickName(), chNoHash));
            return;
        }
        ch->sendTo_all(line, fd);
    } else {
        Client* dest = findClientByNick(target);
        if (!dest) {
            sendResponse(fd, ERR_NOSUCHNICK(sender->GetNickName(), target));
            return;
        }
        sendResponse(dest->GetFd(), line);
    }
}