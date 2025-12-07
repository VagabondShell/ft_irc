#include "../../includes/Server.hpp"
#include <iostream>




void Server::handleTopicCommand(Client *client, std::vector<std::string> args)
{
  if (args.size() < 2) 
  {
    client->SendReply("461", "TOPIC :Not enough parameters");
    return;
  }

  std::string channelName = args[1];
  std::map<std::string, Channel*>::iterator it_ch = _channels.find(channelName);

  if (it_ch == _channels.end()) 
  {
    client->SendReply("403", channelName + " :No such channel");
    return;
  }

  Channel* channel = it_ch->second;

  if (!channel->IsMember(client)) 
  {
    client->SendReply("442", channelName + " :You're not on that channel");
    return;
  }



  if (args.size() == 2)
  {
    if (channel->GetTopic().empty()) 
    {
      client->SendReply("331", channelName + " :No topic is set");
    } 
    else 
    {
      client->SendReply("332", channelName + " " + channel->GetTopic());
      client->SendReply("333", channelName + " " + channel->getTopicSetter() + " " + channel->getTopicSetTime());
    }
    return;
  }
  if (!channel->IsOperator(client) && channel->GetModes().topicOpOnly == true) 
  {
    client->SendReply("482", ":You're not channel operator");
    return;
  }

  std::string newTopic = args[2];

  channel->SetTopic(newTopic);
  channel->setTopicSetter(client->GetNickName());

  time_t now = time(NULL);
  channel->setTopicSetTime(channel->timeToString(now));

  std::string prefix = ":" + client->GetNickName() + "!" + client->GetUserName() + "@" + client->GetIpAddress();
  std::string topicMsg = prefix + " TOPIC " + channelName + " :" + newTopic;
  channel->Broadcast(topicMsg, NULL);
}
