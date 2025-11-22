#include "../../includes/Server.hpp"



void Server::handleJoinCommand(Client *client, std::vector<std::string> args)
{
    if (args.size() < 2)
    {
        client->SendReply("461", "JOIN :Not enough parameters");
        return;
    }

    std::string channelName = args[1];
    std::string key = (args.size() >= 3 ? args[2] : "");

    if (channelName.empty() || channelName[0] != '#')
    {
        client->SendReply("403", channelName + " :No such channel");
        return;
    }

    Channel *channel;

    // Create channel if it does not exist
    if (_channels.find(channelName) == _channels.end())
    {
        channel = new Channel(channelName);
        _channels[channelName] = channel;
    }
    else
        channel = _channels[channelName];

    // ---------- MODE CHECKS ----------
    ChannelModes &m = channel->GetModes();

    if (m.inviteOnly && !channel->IsInvited(client))
    {
        client->SendReply("473", channelName + " :Cannot join channel (+i)");
        return;
    }

    if (m.passwordSet && key != m.password)
    {
        client->SendReply("475", channelName + " :Cannot join channel (+k)");
        return;
    }

    if (m.userLimitSet &&
        (int)channel->GetMembers().size() >= m.userLimit)
    {
        client->SendReply("471", channelName + " :Cannot join channel (+l)");
        return;
    }

    // ---------- JOIN LOGIC ----------
    bool was_empty = channel->GetMembers().empty();

    if (!channel->IsMember(client))
        channel->AddMember(client);

    if (was_empty)
        channel->AddOperator(client);

    // JOIN message
    std::string prefix = ":" + client->GetNickName() + "!" +
                         client->GetUserName() + "@" + client->GetIpAddress();

    std::string joinMsg = prefix + " JOIN " + channelName;

    // Send to this client
    client->GetOutBuffer().append(joinMsg + "\r\n");
    client->SetPollOut(true);

    // Send to others
    channel->Broadcast(joinMsg, client);

    // Send topic
    if (!channel->GetTopic().empty())
    {
        client->GetOutBuffer().append("332 " + client->GetNickName() + " " +
                                      channelName + " :" + channel->GetTopic() + "\r\n");
        client->SetPollOut(true);
    }

    // NAMES list
    std::vector<Client *> members = channel->GetMembers();
    std::string names = "353 " + client->GetNickName() + " = " + channelName + " :";

    for (size_t i = 0; i < members.size(); ++i)
    {
        if (channel->IsOperator(members[i])) names += "@";
        names += members[i]->GetNickName();
        if (i + 1 < members.size()) names += " ";
    }

    client->GetOutBuffer().append(names + "\r\n");
    client->SetPollOut(true);

    client->GetOutBuffer().append("366 " + client->GetNickName() + " " + channelName + " :End of /NAMES list.\r\n");
    client->SetPollOut(true);
}
