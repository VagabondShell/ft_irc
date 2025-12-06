#include "../includes/Server.hpp"

std::string trim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t\n\r");
  if (std::string::npos == first) {
    return str;
  }
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, (last - first + 1));
}

bool Server::isValidNickName(std::string nickname) {
    if (nickname.empty()) {
        return false;
    }
    char first_char = nickname[0];
    if (first_char == '#' || first_char == ':' || std::isdigit(first_char))
        return false;
    
    for (size_t i = 0; i < nickname.size(); i++) {
        char c = nickname[i];
        if (! (std::isalnum(c) || 
               c == '[' || c == ']' || c == '{' || c == '}' || 
               c == '\\' || c == '|')) 
        {
            return false;
        }
    }
    return true; 
}

void Server::handlePassCommand(Client *client, std::vector<std::string>args){
  if (client->IsRegistered()) {
        client->SendReply("462", ":You may not reregister");
        return;
  }
  if (args.size() < 2 || trim(args[1]).empty()) { 
        client->SendReply("461", "PASS :Not enough parameters");
        return; 
  }
  std::string client_password = trim(args[1]);
  if (_password == client_password){
    client->SetPassState(true); 
  }
  else
    client->SendReply("464", ":Password incorrect");
}

void Server::handleNickCommand(Client *client, std::vector<std::string>args){
  if (!client->GetPassState()) {
            client->SendReply("451",":You have not registered");
            return; 
  }
  if (args.size() < 2 || trim(args[1]).empty()) { 
        client->SendReply("431", ":No nickname given");
        return;
    }
  std::string new_nick = trim(args[1]);
  std::string old_nick = client->GetNickName();
  std::cout << "old_nick: " << old_nick << std::endl;
  if (!isValidNickName(new_nick)){
      client->SendReply("432", new_nick + " :Erroneus nickname");
      return;
  }

  std::map<std::string, Client*>::iterator it = _nicknames.find(new_nick);
  if (it != _nicknames.end()) {
    if (it->second == client)
      return; 
    client->SendReply("433", new_nick + " :Nickname is already in use");
    return;
  }
  if (!old_nick.empty()) {
        
         std::string prefix = ":" + old_nick + "!" + client->GetUserName() +
                         "@" + client->GetIpAddress();
        std::string mesg = prefix + " " + "NICK"+" " + new_nick;
        client->GetOutBuffer().append((mesg+"\r\n"));
        client->SetPollOut(true);
        client->BrodcastFromClient(mesg,new_nick);
        _nicknames.erase(old_nick);
  }
  client->SetNickname(new_nick);
  _nicknames[new_nick] = client;
  client->SetNickState(true);
  if (!client->IsRegistered())
      checkRegistration(client); 
}

void Server::handleUserCommand(Client *client, std::vector<std::string>args){
  if (!client->GetPassState()) {
            client->SendReply("451",":You have not registered");
            return; 
  }
  if (client->IsRegistered()) {
    client->SendReply("462", ":You may not reregister");
    return;
  }
  if (args.size() < 2 || trim(args[1]).empty() || args.size() < 5  || (args .size() > 4 && args[4].empty())) { 
    client->SendReply("461", "USER :Not enough parameters");
    return;
  }
  client->SetUserName(args[1]);
  client->SetUserState(true); 
  if (!client->IsRegistered())
      checkRegistration(client); 
}
