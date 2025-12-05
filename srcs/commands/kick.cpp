#include "../../includes/Server.hpp"

void Server::handleKickCommand(Client *client, std::vector<std::string> args)
{
    if (args.size() < 3)
    {
        client->SendReply("461", "KICK :Not enough parameters");
        return;
    }
    std::string channel;
    std::vector<std::string> users;
    std::string comment = " :because i said so";
    std::map<std::string, Channel *>::iterator channel_it;
    std::string prefix = ":" + client->GetNickName() + "!" + client->GetUserName() +
                         "@" + client->GetIpAddress();
    channel = args[1];
    users = generateElements(args[2]);
    if (args.size() > 3)
    {
        comment=" :";
        comment += args[3];
    }
    
    channel_it = _channels.find(channel);
    if (channel_it == _channels.end())
    {
        std::string content = channel + " :No such channel";
        client->SendReply("403", content);
        return;
    }
    if (!channel_it->second->IsOperator(client))
    {
        std::string content = channel + " :You're not channel operator";
        client->SendReply("482", content);
        return;
    }
    for (size_t i = 0; i < users.size(); i++)
    {
        if (!is_active(users[i]))
        {
            std::string content = users[i] + " :No such nick";
            client->SendReply("401", content);
            continue;
        }
        if (!channel_it->second->IsMemberByNick(users[i]))
        {
            std::string content = users[i] + " " + channel + " :They aren't on that channel";
            client->SendReply("441", content);
            continue;
        }
        channel_it->second->Broadcast((prefix + " KICK " + channel + " " + users[i] + comment), NULL);
        
        Client *clientByNick = GetClientByNick(users[i]);
        clientByNick->removeMyChannel(channel_it->second);
        channel_it->second->RemoveMemberByNick(users[i]);
        if (channel_it->second->GetClientCount() == 0)
        {
            Channel *channelToDelete = channel_it->second;
            remove_channel(channel_it->second->GetName());
            delete channelToDelete;
        }
    }
}
