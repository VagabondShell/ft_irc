#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "Server.hpp"
#include <poll.h>
#include <set>
class Client;
class Server;

class Client {
public:

  Client(int Fd, Server* ServerPtr);
  Client(Client &other);
  void ProcessAndExtractCommands();
  std::string ExtractAndEraseFromBuffer(size_t PosFound, int DelimiterLen);

  const std::string GetNickName() const;
  bool GetPassState() const;
  std::string &GetReadBuffer();
  std::string &GetOutBuffer();
  bool IsRegistered() const;
  bool GetUserState() const;
  bool GetNickNameState() const;
  int GetFd() const;
  const std::string GetNickName() ;
  const std::string GetUserName() const;
  const std::string & GetIpAddress() const ;
  void addChannel(Channel *channel);
  void addInvitedChannel(Channel *channel);
  void removeInvitedchannel(Channel *channel);
  void leftAllchannels();
  std::vector<std::string>listOfInvitedChannles();
  Server* GetServerPtr() const;

  void SetRegistration();
  void SetPassState(bool State);
  void SetUserState(bool State);
  void SetNickState(bool State);
  void SetPollOut(bool State);
  void SetIpAddress(const std::string &Addrr);
  void SetNickname(const std::string& Nick);
  void SetUserName(const std::string& User);

  void SendReply(const std::string &Numeric, const std::string &Content);
  void SendPrivateMessage(const std::string &Message);

  void SetInvisible(bool on);
  bool IsInvisible() const;
private:

  int _Fd;
  std::string _ReadBuffer;
  std::string _OutBuffer;

  std::string _NickName;
  std::string _UserName;
  std::string _IpAddrres;

  bool _Registered;
  bool _NickSet;
  bool _PassSet;
  bool _UserSet;

  Server* _ServerPtr;
  std::set<Channel*> mychannles;
  std::set<Channel*> Invited_channel;
  bool _invisible;
};

#endif
