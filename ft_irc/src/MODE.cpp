#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "replies.hpp"

#include <sstream>
#include <vector>
#include <cctype>

static std::string joinArgs(const std::vector<std::string>& v) {
    std::string out;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) out += " ";
        out += v[i];
    }
    return out;
}

static bool parse_positive_int(const std::string& s, int& out) {
    if (s.empty()) return false;
    for (size_t i = 0; i < s.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    std::istringstream iss(s);
    long long v = 0; iss >> v;
    if (v <= 0 || v > 1000000000LL) return false;
    out = static_cast<int>(v);
    return true;
}

void Server::MODE(std::string cmd, int fd)
{
    Client* chi = GetClient(fd);
    if (!chi) return;

    std::istringstream iss(cmd);
    std::string verb, chanTok;
    iss >> verb >> chanTok;          

    const std::string nick = chi->GetNickName();

    if (chanTok.empty()) {
        sendResponse(fd, ERR_NEEDMOREPARAMS(nick, "MODE"));
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

    std::string modeSpec;
    iss >> modeSpec;
    std::vector<std::string> args;
    {
        std::string tmp;
        while (iss >> tmp) args.push_back(tmp);
    }

    if (modeSpec.empty()) {
        sendResponse(fd, RPL_CHANNELMODES(nick, ch->GetName(), ch->getModes()));
        sendResponse(fd, RPL_CREATIONTIME(nick, ch->GetName(), ch->get_creationtime()));
        return;
    }

    {
        std::string me = nick;
        if (!ch->clientInChannel(me)) {
            sendResponse(fd, ERR_NOTONCHANNEL(nick, ch->GetName()));
            return;
        }
    }
    if (!ch->get_admin(fd)) {
        sendResponse(fd, ERR_CHANOPRIVSNEEDED(nick, ch->GetName()));
        return;
    }

    // process modes like "+it-o nick +k key -l"
    char sign = 0;                     // '+' or '-'
    size_t argi = 0;                   // index into args
    std::string outModes;              // what we’ll broadcast (e.g., "+it-k+l")
    std::vector<std::string> outArgs;  // arguments that pair with outModes letters order

    for (size_t i = 0; i < modeSpec.size(); ++i) {
        char c = modeSpec[i];
        if (c == '+' || c == '-') { sign = c; continue; }
        if (sign == 0) {
            sign = '+';
        }

        switch (c) {
            case 'i': { // invite-only
                if (sign == '+') {
                    ch->SetInvitOnly(1);
                    ch->setModeAtindex(0, true);
                } else {
                    ch->SetInvitOnly(0);
                    ch->setModeAtindex(0, false);
                }
                if (outModes.empty() || outModes[outModes.length() - 1] != sign) outModes.push_back(sign);
                outModes.push_back('i');
            } break;

            case 't': { // topic restricted
                if (sign == '+') {
                    ch->set_topicRestriction(true);
                    ch->setModeAtindex(1, true);
                } else {
                    ch->set_topicRestriction(false);
                    ch->setModeAtindex(1, false);
                }
                if (outModes.empty() || outModes[outModes.length() - 1] != sign) outModes.push_back(sign);
                outModes.push_back('t');
            } break;

            case 'k': { // channel key (password)
                if (sign == '+') {
                    if (argi >= args.size()) {
                        sendResponse(fd, ERR_NEEDMODEPARM(ch->GetName(), "+k"));
                        return;
                    }
                    if (!ch->GetPassword().empty()) {
                        sendResponse(fd, ERR_KEYSET(ch->GetName()));
                        break;
                    }
                    const std::string& key = args[argi++];
                    ch->SetPassword(key);
                    ch->setModeAtindex(2, true);

                    if (outModes.empty() || outModes[outModes.length() - 1] != sign) outModes.push_back(sign);
                    outModes.push_back('k');
                    outArgs.push_back(key);
                } else { // -k
                    ch->SetPassword("");
                    ch->setModeAtindex(2, false);

                    if (outModes.empty() || outModes[outModes.length() - 1] != sign) outModes.push_back(sign);
                    outModes.push_back('k');
                }
            } break;

            case 'l': { // user limit
                if (sign == '+') {
                    if (argi >= args.size()) {
                        sendResponse(fd, ERR_NEEDMODEPARM(ch->GetName(), "+l"));
                        return;
                    }
                    int lim = 0;
                    if (!parse_positive_int(args[argi], lim)) {
                        sendResponse(fd, ERR_INVALIDMODEPARM(ch->GetName(), "+l"));
                        return;
                    }
                    ++argi;
                    ch->SetLimit(lim);
                    ch->setModeAtindex(4, true);

                    if (outModes.empty() || outModes[outModes.length() - 1] != sign) outModes.push_back(sign);
                    outModes.push_back('l');

                    std::stringstream ss;
                    ss << lim;
                    outArgs.push_back(ss.str());
                } else { // -l
                    ch->SetLimit(0);
                    ch->setModeAtindex(4, false);

                    if (outModes.empty() || outModes[outModes.length() - 1] != sign) outModes.push_back(sign);
                    outModes.push_back('l');
                }
            } break;

            case 'o': { 
                if (argi >= args.size()) {
                    sendResponse(fd, ERR_NEEDMODEPARM(ch->GetName(), std::string(1, sign) + "o"));
                    return;
                }
                std::string targetNick = args[argi++];
                Client* target = findClientByNick(targetNick);
                if (!target) {
                    sendResponse(fd, ERR_NOSUCHNICK(nick, targetNick));
                    return;
                }
                {
                    std::string tmp = targetNick;
                    if (!ch->clientInChannel(tmp)) {
                        sendResponse(fd, ERR_USERNOTINCHANNEL(nick, targetNick, ch->GetName()));
                        return;
                    }
                }

                if (sign == '+') {
                    ch->change_clientToAdmin(targetNick); 
                } else {
                    ch->change_adminToClient(targetNick);
                }

                if (outModes.empty() || outModes[outModes.length() - 1] != sign) outModes.push_back(sign);
                outModes.push_back('o');
                outArgs.push_back(targetNick);
            } break;

            default: {
                sendResponse(fd, ERR_UNKNOWNMODE(nick, ch->GetName(), std::string(1, c)));
            } break;
        }
    }

    if (!outModes.empty()) {
        const std::string line =
            RPL_CHANGEMODE(chi->getHostname(), ch->GetName(), outModes, joinArgs(outArgs));

        ch->sendTo_all(line);
    }
}