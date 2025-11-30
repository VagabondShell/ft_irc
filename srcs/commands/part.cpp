#include "../../includes/Server.hpp"

void Server::handlePartCommand(Client *client, std::vector<std::string> args)
{
    std::map<std::string, Channel *>::iterator channel_it;
    std::string reason="";
    std::string prefix = ":" + client->GetNickName() + "!" + client->GetUserName() +
    "@" + client->GetIpAddress();
    if (args.size() < 2)
    {
        client->SendReply("461", "PART :Not enough parameters");
        return;
    }
    std::vector<std::string> channels = generateElements(args[1]);
    if (args.size() >= 3)
    {
        reason =" :";
        reason += args[2];
    }
    for (size_t i = 0; i < channels.size(); i++)
    {
        channel_it = _channels.find(channels[i]);
        if (channel_it == _channels.end())
        {
            std::string content = channels[i] + " :No such channel";
            client->SendReply("403", content);
            continue;
        }
        if (!channel_it->second->IsMember(client))
        {
            std::string content = channels[i] + " :You're not on that channel";
            client->SendReply("442", content);
            continue;
        }
        channel_it->second->Broadcast(prefix +" PART "+channels[i]+reason);
        channel_it->second->RemoveMember(client);
        if(channel_it->second->GetClientCount() == 0)
        {
            Channel* channelToDelete = channel_it->second;
            client->removeMyChannel(channelToDelete);
            _channels.erase(channels[i]);
            delete channelToDelete;
        }
    }
}