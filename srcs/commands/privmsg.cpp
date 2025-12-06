#include "../../includes/Server.hpp"


void Server::handlePrivmsgCommand(Client *client, std::vector<std::string> args)
{
  if (args.size() < 2)
  {
    client->SendReply("411", ":No recipient given (" + args[0] + ")");
    return;
  }
  if (args.size() < 3 || (args.size() > 2 && args[2].empty()))
  {
    client->SendReply("412", ":No text to send");
    return;
  }
  std::string message = args[2];
  std::vector<std::string> recipients = generateElements(args[1]);
  for (size_t i = 0; i < recipients.size(); i++){
    std::string target = recipients[i];
    if (target[0] == '#') 
    {
      std::map<std::string, Channel*>::iterator chan_list = _channels.find(target);
      if (chan_list == _channels.end()) {
        client->SendReply("403", target + " :No such channel");
        return;
      }

      Channel *channel = chan_list->second;

      if (!channel->IsMember(client)) {
        client->SendReply("442", target + " :You're not on that channel");
        return;
      }

      std::string prefix = ":" + client->GetNickName() + "!" + client->GetUserName() +
        "@" + client->GetIpAddress();

      std::string msg = prefix + " PRIVMSG " + target + " :" + message;

      channel->Broadcast(msg, client);
      return;
    }


    std::map<std::string, Client*>::iterator it = _nicknames.find(target);
    if (it == _nicknames.end())
    {
      client->SendReply("401", target + " :No such nick/channel");
      return;
    }
    Client *receiver = it->second;
    if (!receiver->IsRegistered())
    {
      client->SendReply("401", target + " :No such nick/channel");
      return;
    }
    std::string prefix = ":" + client->GetNickName() + "!" +
      client->GetUserName() + "@" +
      client->GetIpAddress();

    std::string msg = prefix + " PRIVMSG " + target + " :" + message + "\r\n";
    if (send(receiver->GetFd(), msg.c_str(), msg.length(), 0) == -1)
      std::cerr << "send() failed to client" << std::endl;

  }



  return;
}
