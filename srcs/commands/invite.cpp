#include "../../includes/Server.hpp"
void PrintIinviteLIst(Client *client)
{
    std::vector<std::string> list = client->listOfInvitedChannles();

    std::string message;
    for (size_t i = 0; i < list.size(); i++)
    {
        message =" " + list[i];
        client->SendReply("336", message);
    }
    client->SendReply("337"," :End of INVITE list");
}
void Server::handleInviteCommand(Client *client, std::vector<std::string> args)
{
    if (args.size() == 1)
    {
        PrintIinviteLIst(client);
        return;
    }
    else if (args.size() < 3)
    {
        client->SendReply("461", "INVITE :Not enough parameters");
        return;
    }
    ChannelModes Mods;
    std::string channel;
    std::string target;
    std::map<std::string, Channel *>::iterator channel_it;
    std::string prefix = ":" + client->GetNickName() + "!" + client->GetUserName() +
                         "@" + client->GetIpAddress();
    channel = args[2];
    target = args[1];
    channel_it = _channels.find(channel);
    
    if (channel_it == _channels.end())
    {
        std::string content = channel + " :No such channel";
        client->SendReply("403", content);
        return;
    }
    if (!channel_it->second->IsMember(client))
    {
        std::string content =channel + " :You're not on that channel";
        client->SendReply("442", content);
        return;
    }
    if (channel_it->second->IsMemberByNick(target))
    {
        std::string content = target + " " + channel + " :is already on channel";
        client->SendReply("443", content);
        return;
    }
    Mods = channel_it->second->GetModes();
    if (!channel_it->second->IsOperator(client) && Mods.inviteOnly)
    {
        std::string content = channel + " :You're not channel operator";
        client->SendReply("482", content);
        return;
    }
 
    if (!is_active(target))
    {
        std::string content =  target + " :No such nick";
        client->SendReply("401", content);
        return;
    }
    channel_it->second->InviteMember(target);
    std::string content =target + " " + channel;
    client->SendReply("341", content);
    Client *targetClient = GetClientByNick(target);
    if (targetClient)
    {
        std::string inviteMsg = prefix + " INVITE " + target + " " + channel;
        targetClient->addInvitedChannel(channel_it->second->GetName());
        targetClient->GetOutBuffer().append(inviteMsg + "\r\n");
        targetClient->SetPollOut(true);
    }
    return;
}