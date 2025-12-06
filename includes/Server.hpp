#ifndef SERVER_HPP
#define SERVER_HPP

#define GREEN    "\x1b[32m" 
#define RED      "\x1b[31m"
#define BOT_NAME "FTBot"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>
#include <string>
#include "Command.hpp"
#include "Channel.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>
#include "Client.hpp"
#include "Command.hpp"
#include "Command.hpp"
#include <csignal>

class Client;
class Channel;

class Server {
public:

  Server(const int port, const std::string password);
  ~Server();
  void run();
  void commandDispatcher(Client *client, std::string commandLine);
  void checkRegistration(Client * client);
  bool handleOutgoingData(int clientFd);
  bool isValidNickName(std::string  nickname);
  e_cmd_type getCommandType(std::string command);
  std::vector<struct pollfd> & getPollfds();
  void disconnectClient(int currentFd);
  void handlePrivmsgCommand(Client *client, std::vector<std::string>args); 
  void clearChannel(Channel *channel);
  void handleModeCommand(Client *client, std::vector<std::string> args);
  void handleJoinCommand(Client *client, std::vector<std::string> args);
  void handleInviteCommand(Client *client, std::vector<std::string> args);
  void handleKickCommand(Client *client, std::vector<std::string> args);
  void handleTopicCommand(Client *client, std::vector<std::string> args);
  void handlePartCommand(Client *client, std::vector<std::string> args);
  bool is_active(std::string);
  time_t getStartTime() const ;
  void remove_channel(std::string channelName);
  Client *GetClientByNick(std::string nick);
  std::map<std::string, Client *> GetNickNames() const;
  
  int execute_modes(Client* client, const std::string& channelName, const std::vector<std::string>& modes, const std::vector<std::string>& modeParams);

private:

  void handleNewConnection();
  bool handleClientCommand(const int currentFd);
  //Authentication commands 
  void handlePassCommand(Client *client, std::vector<std::string>args); 
  void handleUserCommand(Client *client, std::vector<std::string>args); 
  void handleNickCommand(Client *client, std::vector<std::string>args);
 
  std::string _password;
  int _port;
  int _listenerFd;
  std::string _serverName;
  std::vector<struct pollfd> _pollFds;
  std::map<int, Client *> _clients;
  std::map<std::string, Client *> _nicknames;
  std::map<std::string, e_cmd_type> _commandMap;
  std::map<std::string, Channel *> _channels;

  time_t _StartTime;

};
void processBotCommand(Client * client, std::string & message);
std::vector<std::string> split_string_to_vector(const std::string &input_string,
    char delimiter);
bool check_channel(std::string channel);
std::vector<std::string> generateElements(std::string str);
#endif
