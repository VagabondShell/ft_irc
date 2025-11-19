#include "../includes/Server.hpp"

void Server::handlePrivmsgCommand(Client *client, std::vector<std::string>args){
  if (args.size() < 2) { 
    client->SendReply("411", ":No recipient given (" + args[0] + ")");
    return; 
  }
  if (args.size() < 3) {
    client->SendReply("412", ":No text to send");
    return;
  }
  std::string target = args[1];
  std::string message = args[2];
  if (target == BOT_NAME)
  {
    std::map<std::string, Client*>::iterator it = _nicknames.find(BOT_NAME);
    if (it != _nicknames.end()) {
      std::string botMessage = client->GetNickName() + " " + message + "\r\n";
      if (send(it->second->GetFd(), botMessage.c_str(), botMessage.length(), 0) == -1){
        throw std::runtime_error("Bot connection failed during send.");
        return;
      }
    }
    else {
      std::cerr << "[ERROR] Bot not found in map, skipping command dispatch." << std::endl;
      client->SendReply("401", "bot :No such nick/channel");
      return;
    }
  }
  else
  {
    std::map<std::string, Client*>::iterator it = _nicknames.find(target);
    if (it != _nicknames.end()) {
      Client *senderClient = it->second; 
      std::string messageee =  "PRIVMSG " + senderClient->GetNickName() + " :" + message + "\r\n" ;
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
    if (args.size() < 3) {
        client->SendReply("461", "MODE :Not enough parameters");
        return;
    }
    std::string target = args[1];
    std::string modes = args[2];

    if (!target.empty() && target[0] == '#') {
        client->SendReply("403", target + " :Channel MODE not supported");
        return;
    }

    std::map<std::string, Client*>::iterator it = _nicknames.find(target);
    if (it == _nicknames.end()) {
        client->SendReply("401", target + " :No such nick/channel");
        return;
    }

    Client* targetClient = it->second;
    if (client != targetClient) {
        client->SendReply("482", ":You do not have permission to change other users' modes");
        return;
    }

    bool set_mode = true;
    for (size_t i = 0; i < modes.size(); ++i) {
        char c = modes[i];
        if (c == '+') { set_mode = true; continue; }
        if (c == '-') { set_mode = false; continue; }
        if (c == 'i') {
            targetClient->SetInvisible(set_mode);
            continue;
        }

    }


    client->SendReply("221", target + " " + modes);

    std::string modeNotification = ":ft_irc.local MODE " + target + " " + modes;
    targetClient->GetOutBuffer().append(modeNotification + "\r\n");
    targetClient->SetPollOut(true);
}