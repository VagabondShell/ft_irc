#include "../../includes/Server.hpp"
#include <iostream>



void Server::handleTopicCommand(Client *client, std::vector<std::string> args)
{
  std::cout << "The size of the vector is: " << args.size() << std::endl;

  for (size_t i = 0; i < args.size(); i++)
  {
    if (!args[i].empty())
      std::cout << args[i] << std::endl;
    else
      std::cout << RED << "Empty parameters" << std::endl;
  }
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

  if (!channel->IsOperator(client) && channel->GetModes().topicOpOnly == true) {
    client->SendReply("482", ":You're not channel operator");
    return;
  }

  channel->SetTopic(newTopic);

  std::string prefix = ":" + client->GetNickName() + "!" + client->GetUserName() + "@" + client->GetIpAddress();
  std::string topicMsg = prefix + " TOPIC " + channelName + " :" + newTopic;
  channel->Broadcast(topicMsg, NULL);

  client->SendReply("332", channelName + " :" + newTopic);
}
