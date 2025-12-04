#include "../includes/Client.hpp"
#include <vector>
#include <set>  
Client::Client(Client &other)
{
    _Fd          = other._Fd;
    _ReadBuffer  = other._ReadBuffer;
    _OutBuffer   = other._OutBuffer;

    _NickName    = other._NickName;
    _UserName    = other._UserName;
    _IpAddrres   = other._IpAddrres;

    _Registered  = other._Registered;
    _NickSet     = other._NickSet;
    _PassSet     = other._PassSet;
    _UserSet     = other._UserSet;

    _ServerPtr   = other._ServerPtr;   // shallow copy (pointer)
    _invisible   = other._invisible;
}

Client::Client(int fd, Server* serverPtr)
    : _Fd(fd), _OutBuffer(""), _Registered(false), _NickSet(false),
      _PassSet(false), _UserSet(false), _ServerPtr(serverPtr), _invisible(false) {}

//Get the commnad untile the /r/n return it and remove it from the buffer
std::string Client::ExtractAndEraseFromBuffer(size_t pos_found, int dilimiterLen) {
  std::string toRetrun = _ReadBuffer.substr(0, pos_found);
  _ReadBuffer = _ReadBuffer.substr(pos_found + dilimiterLen);
  return toRetrun;
}

void Client::SendReply(const std::string &numeric,
                        const std::string &content) {
  std::string prefix = ":ft_irc.local";
  std::string recipient;
  
  if (this->IsRegistered() || !this->GetNickName().empty()) {
    recipient = this->GetNickName();
  } else {
    recipient = "*"; 
  }

  std::string full_message = prefix + " " + numeric + " " + recipient + " " + content;
  std::cout << "full message: " << full_message << std::endl;
  this->GetOutBuffer().append(full_message + "\r\n");
  this->SetPollOut(true);
}

void Client::SendPrivateMessage(const std::string &Message){
    std::string FullMessage =  "PRIVMSG " + GetNickName() + " :" + Message + "\r\n" ;
    this->GetOutBuffer().append(FullMessage + "\r\n");
    this->SetPollOut(true);
}

//Getters
Server* Client::GetServerPtr() const{
  return _ServerPtr;
}

std::string &Client::GetReadBuffer() {
  return _ReadBuffer; 
}

int Client::GetFd() const{
  return _Fd;
}

bool Client::IsRegistered() const {
  return _Registered; 
}

bool Client::GetPassState() const{
  return _PassSet;
}

bool Client::GetUserState() const{
  return _UserSet;
}

const std::string Client::GetNickName() const {
  return _NickName; 
}

const std::string Client::GetUserName() const {
  return _UserName; 
}

std::string & Client::GetOutBuffer(){
    return _OutBuffer;
}

bool Client::GetNickNameState() const{
  return _NickSet;
}

const std::string & Client::GetIpAddress() const{
  return _IpAddrres;
}

//Setters
void Client::SetNickname(const std::string& Nick){
  _NickName = Nick; 
}
void Client::SetUserName(const std::string& User){
  _UserName = User;
}

void Client::SetRegistration(){
  _Registered = true;
}

void Client::SetNickState(bool state){
  _NickSet = state;
}

void Client::SetPassState(bool state){
  _PassSet = state;
}

void Client::SetUserState(bool state){
  _UserSet = state;
}

void Client::SetIpAddress(const std::string &addrr){
  _IpAddrres = addrr;
}


void Client::SetInvisible(bool on) {
    _invisible = on;
}

bool Client::IsInvisible() const {
    return _invisible;
}

void Client::SetPollOut(bool state){
    std::vector<struct pollfd>& poll_fds = _ServerPtr->getPollfds();
    for (size_t i = 0; i < poll_fds.size(); ++i) {

        if (poll_fds[i].fd == _Fd) {
            if (state == true) {
                poll_fds[i].events |= POLLOUT; 
            } else {
                poll_fds[i].events &= ~POLLOUT;
            }
            return; 
        }
    }
    //TODO If the loop finishes without finding the FD, the client may have disconnected.
}
    
// retreave the commmand and its argument then run it 
void Client::ProcessAndExtractCommands() {
  int dilimiterLen;
  size_t pos_found = _ReadBuffer.find("\r\n");
  dilimiterLen = 2;
  if (pos_found == std::string::npos)
  {
    pos_found = _ReadBuffer.find("\n");
    dilimiterLen = 1;
  }

  while (pos_found != std::string::npos) {
    std::string command_line = ExtractAndEraseFromBuffer(pos_found, dilimiterLen);
    _ServerPtr->commandDispatcher(this, command_line);
    pos_found = _ReadBuffer.find("\r\n");
    if (pos_found == std::string::npos)
      pos_found = _ReadBuffer.find("\n");
  }
}

void Client::leftAllchannels()
{
    std::set<Channel*>::iterator it = mychannles.begin();
    while (it != mychannles.end())
    {
        Channel *chan = *it;
        chan->RemoveMember(this);
        if (chan->GetClientCount() == 0)
        {
            this->_ServerPtr->remove_channel(chan->GetName());
            delete chan;
        }
        mychannles.erase(it++);
    }
}


void Client::BrodcastFromClient(std::string msg)
{
    std::set<Channel*>::iterator it = mychannles.begin();
    std::map<std::string,Client*> listclient;
    std::set<Client*> my_friends;
    while (it != mychannles.end())
    {
        Channel *chan = *it;
        listclient = chan->GetMembers();
        for (std::map<std::string,Client*>::iterator it2 = listclient.begin(); it2 != listclient.end() ; it2++)
        {
          my_friends.insert(it2->second);
        }        
        it++;
    }
       for (std::set<Client*>::iterator it = my_friends.begin(); it != my_friends.end(); ++it)
      {
        if (*it != this) {
            (*it)->GetOutBuffer().append(msg + "\r\n");
            (*it)->SetPollOut(true);
        }
      }
}
std::vector<std::string> Client::listOfInvitedChannles()
{
    std::vector<std::string> list;
    std::set<Channel*>::iterator it = Invited_channel.begin();
    while (it != Invited_channel.end())
    {
       list.push_back((*it)->GetName());
       it++;
    }
    return list;
}
void Client::addChannel(Channel *channel)
{
    mychannles.insert(channel);
}
void Client::addInvitedChannel(Channel *channel)
{
  Invited_channel.insert(channel);
}
void Client::removeInvitedchannel(Channel *channel)
{
  Invited_channel.erase(channel);
}
void Client::removeMyChannel(Channel *channel)
{
  mychannles.erase(channel);
}
const std::set<Channel*>& Client::GetClientChannels() const
{
  return mychannles;
}
