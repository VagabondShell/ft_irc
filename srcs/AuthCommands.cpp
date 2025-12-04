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
    //TODO check if this line is correct
    char first_char = nickname[0];
    if (nickname.find(' ') != std::string::npos) {
        return false;
    }
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
  if (args.size() < 2 || trim(args[1]).empty()) { 
        client->SendReply("461", "PASS :Not enough parameters");
        return; 
  }
  if (client->IsRegistered()) {
        client->SendReply("462", ":You may not reregister");
        return;
  }
  std::string client_password = trim(args[1]);
  std::cout << "the password the client send *" << client_password << "*" <<  std::endl;
  if (_password == client_password){
    client->SetPassState(true); 
    std::cout << GREEN 
      << "[SUCCESS] " << " authenticated password successfully." << std::endl;
  }
  else
    client->SendReply("464", ":Password incorrect");
}

void Server::handleNickCommand(Client *client, std::vector<std::string>args){
  if (args.size() < 2 || trim(args[1]).empty()) { // Check argument existence AND trimmed emptiness
        client->SendReply("431", ":No nickname given");
        return;
    }
  // if (client->IsRegistered()) {
  //       client->SendReply("462", ":You may not reregister");
  //       return;
  // }
  std::string new_nick = trim(args[1]);
  std::string old_nick = client->GetNickName();
  std::cout << "old_nick: " << old_nick << std::endl;
  if (!isValidNickName(new_nick)){
      client->SendReply("432", new_nick + ":Erroneus nickname");
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
        _nicknames.erase(old_nick);
        // Logic to broadcast the NICK change message to network
         std::string prefix = ":" + client->GetNickName() + "!" + client->GetUserName() +
                         "@" + client->GetIpAddress();
        std::string mesg = prefix + " " + "NICK"+" " + new_nick;
        client->GetOutBuffer().append((mesg+"\r\n"));
        client->SetPollOut(true);
        client->BrodcastFromClient(mesg);
  }
  client->SetNickname(new_nick) ;
  _nicknames[new_nick] = client;
  client->SetNickState(true);
  if (!client->IsRegistered())
      checkRegistration(client); 
}

void Server::handleUserCommand(Client *client, std::vector<std::string>args){
  if (args.size() < 4 || trim(args[1]).empty()) { 
    client->SendReply("461", ":Not enough parameters");
    return;
  }
  if (client->IsRegistered()) {
    client->SendReply("462", ":You may not reregister");
    return;
  }
  client->SetUserName(args[1]);
  std::cout << "_userName: " << client->GetUserName() << std::endl;
  client->SetUserState(true); 
  std::cout << GREEN 
    << "[SUCCESS] " << " User  successfully." << std::endl;
  if (!client->IsRegistered())
      checkRegistration(client); 
}
