#include "../includes/Server.hpp"

void Server::handlePrivmsgCommand(Client *client, std::vector<std::string> args)
{
  if (args.size() < 2)
  {
    client->SendReply("411", ":No recipient given (" + args[0] + ")");
    return;
  }
  if (args.size() < 3)
  {
    client->SendReply("412", ":No text to send");
    return;
  }
  std::string target = args[1];
  std::string message = args[2];
  if (target == BOT_NAME)
  {
    std::map<std::string, Client *>::iterator it = _nicknames.find(BOT_NAME);
    if (it != _nicknames.end())
    {
      std::string botMessage = client->GetNickName() + " " + message + "\r\n";
      if (send(it->second->GetFd(), botMessage.c_str(), botMessage.length(), 0) == -1)
      {
        throw std::runtime_error("Bot connection failed during send.");
        return;
      }
    }
    else
    {
      std::cerr << "[ERROR] Bot not found in map, skipping command dispatch." << std::endl;
      client->SendReply("401", "bot :No such nick/channel");
      return;
    }
  }
  else
  {
    std::map<std::string, Client *>::iterator it = _nicknames.find(target);
    if (it != _nicknames.end())
    {
      Client *senderClient = it->second;
      std::string messageee = "PRIVMSG " + senderClient->GetNickName() + " :" + message + "\r\n";
      std::cout << "outgoing message: " << messageee << std::endl;
      send(senderClient->GetFd(), messageee.c_str(), messageee.length(), 0);
      return;
    }
    else
    {
      std::cout << "no client found" << std::endl;
      return;
    }
  }
}

void Server::handleModeCommand(Client *client, std::vector<std::string> args)
{
  if (args.size() < 3)
  {
    client->SendReply("461", "MODE :Not enough parameters");
    return;
  }
  std::string target = args[1];
  std::string modes = args[2];

  if (!target.empty() && target[0] == '#')
  {
    client->SendReply("403", target + " :Channel MODE not supported");
    return;
  }

  std::map<std::string, Client *>::iterator it = _nicknames.find(target);
  if (it == _nicknames.end())
  {
    client->SendReply("401", target + " :No such nick/channel");
    return;
  }

  Client *targetClient = it->second;
  if (client != targetClient)
  {
    client->SendReply("482", ":You do not have permission to change other users' modes");
    return;
  }

  bool set_mode = true;
  for (size_t i = 0; i < modes.size(); ++i)
  {
    char c = modes[i];
    if (c == '+')
    {
      set_mode = true;
      continue;
    }
    if (c == '-')
    {
      set_mode = false;
      continue;
    }
    if (c == 'i')
    {
      targetClient->SetInvisible(set_mode);
      continue;
    }
  }

  client->SendReply("221", target + " " + modes);

  std::string modeNotification = ":ft_irc.local MODE " + target + " " + modes;
  targetClient->GetOutBuffer().append(modeNotification + "\r\n");
  targetClient->SetPollOut(true);
}

void Server::handleJoinCommand(Client *client, std::vector<std::string> args)
{
  if (args.size() < 2)
  {
    client->SendReply("461", "JOIN :Not enough parameters");
    return;
  }

  std::string channelName = args[1];
  if (channelName.empty() || channelName[0] != '#')
  {
    client->SendReply("403", channelName + " :No such channel");
    return;
  }

  Channel *channel = NULL;
  std::map<std::string, Channel *>::iterator it = _channels.find(channelName);
  if (it == _channels.end())
  {
    channel = new Channel(channelName);
    _channels[channelName] = channel;
  }
  else
  {
    channel = it->second;
  }

  bool was_empty = channel->GetMembers().empty();
  if (!channel->IsMember(client))
  {
    channel->AddMember(client);
    if (was_empty)
    {
      channel->AddOperator(client);
    }
  }

  std::string prefix = ":" + client->GetNickName() + "!" + client->GetUserName() + "@" + client->GetIpAddress();
  std::string joinMsg = prefix + " JOIN " + channelName;

  client->GetOutBuffer().append(joinMsg + "\r\n");
  client->SetPollOut(true);

  channel->Broadcast(joinMsg, client);

  if (!channel->GetTopic().empty())
  {
    client->GetOutBuffer().append("332 " + client->GetNickName() + " " + channelName + " :" + channel->GetTopic() + "\r\n");
    client->SetPollOut(true);
  }

  std::vector<Client *> members = channel->GetMembers();
  std::string namesLine = "353 " + client->GetNickName() + " = " + channelName + " :";
  for (size_t i = 0; i < members.size(); ++i)
  {
    if (channel->IsOperator(members[i]))
      namesLine += "@";
    namesLine += members[i]->GetNickName();
    if (i + 1 < members.size())
      namesLine += " ";
  }
  client->GetOutBuffer().append(namesLine + "\r\n");
  client->SetPollOut(true);
  client->GetOutBuffer().append("366 " + client->GetNickName() + " " + channelName + " :End of /NAMES list.\r\n");
  client->SetPollOut(true);
}

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

void Server::handleTopicCommand(Client *client, std::vector<std::string> args)
{
  if (args.size() < 2) {
    client->SendReply("461", "TOPIC :Not enough parameters");
    return;
  }

  std::string channelName = args[1];
  std::map<std::string, Channel*>::iterator it_ch = _channels.find(channelName);
  if (it_ch == _channels.end()) {
    client->SendReply("403", channelName + " :No such channel");
    return;
  }
  Channel* channel = it_ch->second;

  if (args.size() == 2) {
    if (channel->GetTopic().empty()) {
      client->SendReply("331", channelName + " :No topic is set");
    } else {
      client->SendReply("332", channelName + " :" + channel->GetTopic());
    }
    return;
  }

  std::string newTopic = args[2];

  if (!channel->IsMember(client)) {
    client->SendReply("442", channelName + " :You're not on that channel");
    return;
  }

  if (!channel->IsOperator(client)) {
    client->SendReply("482", ":You're not channel operator");
    return;
  }

  channel->SetTopic(newTopic);

  std::string prefix = ":" + client->GetNickName() + "!" + client->GetUserName() + "@" + client->GetIpAddress();
  std::string topicMsg = prefix + " TOPIC " + channelName + " :" + newTopic;
  channel->Broadcast(topicMsg, NULL);

  client->SendReply("332", channelName + " :" + newTopic);
}