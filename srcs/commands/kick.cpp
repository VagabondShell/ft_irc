#include "../../includes/Server.hpp"
std::vector<std::string> genrateChannel_user(std::string str)
{
    std::vector<std::string> elmnts;
    std::string elm = "";
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == ' ')
            break;
        if (str[i] == ',')
        {
            if (elm.find(' ') == std::string::npos)
                elmnts.push_back(elm);
            elm = "";
        }
        else
            elm += str[i];
    }
    if ((elm.find(' ') == std::string::npos && !elm.empty()))
        elmnts.push_back(elm);

    return elmnts;
}
void Server::handleKickCommand(Client *client, std::vector<std::string> args)
{
    if (args.size() < 3)
    {
        client->SendReply("461", "KICK :Not enough parameters");
        return;
    }
    std::string channel;
    std::vector<std::string> users;
    std::string comment = ":default";
    std::map<std::string, Channel *>::iterator channel_it;
    std::string prefix = ":" + client->GetNickName() + "!~" + client->GetUserName() +
                         "@" + client->GetIpAddress();
    channel = args[1];
    users = genrateChannel_user(args[2]);
    if (args.size() > 3)
        comment = args[3];

    channel_it = _channels.find(channel);
    if (channel_it == _channels.end() && check_channel(channel))
    {
        std::string content = channel + " :No such channel";
        client->SendReply("403", content);
        return;
    }
    else if (!check_channel(channel))
    {
        std::string content = channel + " :Bad Channel Mask";
        client->SendReply("476", content);
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
            return;
        }
        if (!channel_it->second->IsMemberByNick(users[i]))
        {
            std::string content = users[i] + " " + channel + " :They aren't on that channel";
            client->SendReply("441", content);
            return;
        }
        channel_it->second->RemoveMemberByNick(users[i]);
        channel_it->second->Broadcast((prefix + " KICK " + channel + " " + users[i] + " " + comment), NULL);
        if (channel_it->second->GetClientCount() == 0)
            remove_channel(channel_it->second->GetName());
    }
}
