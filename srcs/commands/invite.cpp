#include "../../includes/Server.hpp"



void Server::handleInviteCommand(Client *client, std::vector<std::string> args)
{
  if (args.size() < 3)
  {
    client->SendReply("461", "INVITE :Not enough parameters");
    return;
  }

  std::string targetNick = args[1];
  std::string channelName = args[2];

  std::map<std::string, Client *>::iterator it_target = _nicknames.find(targetNick);
  if (it_target == _nicknames.end())
  {
    client->SendReply("401", targetNick + " :No such nick/channel");
    return;
  }
  Client *targetClient = it_target->second;

  std::map<std::string, Channel *>::iterator it_ch = _channels.find(channelName);
  if (it_ch == _channels.end())
  {
    client->SendReply("403", channelName + " :No such channel");
    return;
  }
  Channel *channel = it_ch->second;

  if (!channel->IsMember(client))
  {
    client->SendReply("442", channelName + " :You're not on that channel");
    return;
  }

  if (!channel->IsOperator(client))
  {
    client->SendReply("482", ":You're not channel operator");
    return;
  }

  channel->InviteMember(targetClient);

  client->SendReply("341", client->GetNickName() + " " + targetNick + " " + channelName);

  std::string prefix = ":" + client->GetNickName() + "!" + client->GetUserName() + "@" + client->GetIpAddress();
  std::string inviteMsg = prefix + " INVITE " + targetNick + " :" + channelName;
  targetClient->GetOutBuffer().append(inviteMsg + "\r\n");
  targetClient->SetPollOut(true);
}