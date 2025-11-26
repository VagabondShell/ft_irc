#include "../../includes/Server.hpp"


void Server::handleModeCommand(Client *client, std::vector<std::string> args)
{
    if (args.size() < 3) {
        client->SendReply("461", "MODE :Not enough parameters");
        return;
    }

    std::string target = args[1];
    std::string modes = args[2];


    if (!target.empty() && target[0] == '#') {
        std::map<std::string, Channel*>::iterator it = _channels.find(target);
        if (it == _channels.end()) {
            client->SendReply("403", target + " :No such channel");
            return;
        }

        Channel *channel = it->second;

        if (!channel->IsOperator(client)) {
            client->SendReply("482", target + " :You're not channel operator");
            return;
        }

        bool set_mode = true;
        size_t paramIndex = 3;

        for (size_t i = 0; i < modes.size(); ++i) {
            char c = modes[i];
            if (c == '+') { set_mode = true; continue; }
            if (c == '-') { set_mode = false; continue; }

            if (c == 'i') {
                channel->GetModes().inviteOnly = set_mode;
            }
            else if (c == 't') {
                channel->GetModes().topicOpOnly = set_mode;
            }
            else if (c == 'k') {
                if (set_mode && args.size() > paramIndex)
                    channel->GetModes().password = args[paramIndex++];
                channel->GetModes().passwordSet = set_mode;
            }
            else if (c == 'l') {
                if (set_mode && args.size() > paramIndex)
                    channel->GetModes().userLimit = atoi(args[paramIndex++].c_str());
                channel->GetModes().userLimitSet = set_mode;
            }
            else if (c == 'o') {
                if (args.size() > paramIndex) {
                    std::map<std::string, Client*>::iterator itClient = _nicknames.find(args[paramIndex++]);
                    if (itClient != _nicknames.end()) {
                        if (set_mode) channel->AddOperator(itClient->second);
                        else channel->RemoveOperator(itClient->second);
                    }
                }
            }
        }

        std::string notify = ":ft_irc.local MODE " + target + " " + modes;
        channel->Broadcast(notify, NULL);
        return;
    }


    std::map<std::string, Client*>::iterator itUser = _nicknames.find(target);
    if (itUser == _nicknames.end()) {
        client->SendReply("401", target + " :No such nick/channel");
        return;
    }

    Client *targetClient = itUser->second;
    if (client != targetClient) {
        client->SendReply("482", ":You do not have permission to change other users' modes");
        return;
    }

    bool set_mode = true;
    for (size_t i = 0; i < modes.size(); ++i) {
        char c = modes[i];
        if (c == '+') { set_mode = true; continue; }
        if (c == '-') { set_mode = false; continue; }
        if (c == 'i') targetClient->SetInvisible(set_mode);
    }

    client->SendReply("221", target + " " + modes);
}
