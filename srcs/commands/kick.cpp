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
    std::vector<std::string> channels;
    std::vector<std::string> users;
    std::string comment ="default";
    std::map<std::string, Channel *>::iterator channel_it;
    std::string prefix = ":" + client->GetNickName() + "!~" + client->GetUserName() +
    "@" + client->GetIpAddress();
    channels = genrateChannel_user(args[1]);
    users = genrateChannel_user(args[2]);
    if (args.size() > 4)
        comment = args[3];
    for (size_t i = 0; i < channels.size(); i++)
    {
        channel_it = _channels.find(channels[i]);
        if (channel_it == _channels.end())
        {
            std::string content = channels[i] + " :No such channel";
            client->SendReply("403", content);
            continue;
        }
        if (!channel_it->second->IsOperator(client))
        {
            std::string content = channels[i] + " :You're not channel operator";
            client->SendReply("482", content);
            continue;
        }
        if(!channel_it->second->IsMemberByNick(users[i]))
        {
            std::string content = users[i]+" "+channels[i] + " :They aren't on that channel";
            client->SendReply("441", content);
            continue;
        }
        if(!is_active(users[i]))
        {
            std::string content = users[i]+" "+" :No such nick";
            client->SendReply("401", content);
        }
        channel_it->second->Broadcast((prefix + "KICK" + channels[i] + " " + users[i] + " " + comment+ "\r\n"),client);
    }
}
