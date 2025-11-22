#include "../../includes/Server.hpp"



void Server::handleKickCommand(Client *client, std::vector<std::string> args)
{
  if (args.size() < 3)
  {
    client->SendReply("461", "KICK :Not enough parameters");
    return;
  }

  std::string channelName = args[1];
  std::string targetNick = args[2];
  std::string reason = "";
  if (args.size() >= 4)
    reason = args[3];

  std::map<std::string, Channel *>::iterator it_ch = _channels.find(channelName);
  if (it_ch == _channels.end())
  {
    client->SendReply("403", channelName + " :No such channel");
    return;
  }
  Channel *channel = it_ch->second;

  std::map<std::string, Client *>::iterator it_target = _nicknames.find(targetNick);
  if (it_target == _nicknames.end())
  {
    client->SendReply("401", targetNick + " :No such nick/channel");
    return;
  }
  Client *targetClient = it_target->second;

  if (!channel->IsOperator(client))
  {
    client->SendReply("482", ":You're not channel operator");
    return;
  }

  if (!channel->IsMember(targetClient))
  {
    client->SendReply("441", targetNick + " " + channelName + " :They aren't on that channel");
    return;
  }

  std::string prefix = ":" + client->GetNickName() + "!" + client->GetUserName() + "@" + client->GetIpAddress();
  std::string kickMsg = prefix + " KICK " + channelName + " " + targetNick;
  if (!reason.empty())
    kickMsg += " :" + reason;

  std::vector<Client *> members = channel->GetMembers();
  for (size_t i = 0; i < members.size(); ++i)
  {
    members[i]->GetOutBuffer().append(kickMsg + "\r\n");
    members[i]->SetPollOut(true);
  }

  channel->RemoveMember(targetClient);
  channel->RemoveOperator(targetClient);

  if (channel->GetMembers().empty())
  {
    delete channel;
    _channels.erase(channelName);
  }
}

