#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"

Channel::Channel(const std::string& name)
    : _name(name), _topic(""), _modes() {}

const std::string& Channel::GetName() const {
    return _name;
}

const std::string& Channel::GetTopic() const {
    return _topic;
}

void Channel::SetTopic(const std::string& topic) {
    _topic = topic;
}

bool Channel::IsMember(Client* client) const {
    for (std::map<std::string, Client*>::const_iterator it = _members.begin(); it != _members.end(); ++it) {
        if (it->second == client)
            return true;
    }
    return false;
}
bool Channel::IsMemberByNick(std::string nick) const {
   return _members.find(nick) != _members.end();
}

bool Channel::IsOperator(Client* client) const {
    for (std::map<std::string, Client*>::const_iterator it = _operators.begin(); it != _operators.end(); ++it) {
        if (it->second == client)
            return true;
    }
    return false;
}

void Channel::AddMember(Client* client) {
    _members[client->GetNickName()] = client;
}

void Channel::RemoveMember(Client* client) {
    _members.erase(client->GetNickName());
    _operators.erase(client->GetNickName());
}
void Channel::RemoveMemberByNick(std::string nick) {
    _members.erase(nick);
    _operators.erase(nick);
}
void Channel::AddOperator(Client* client) {
    if (IsMember(client))
        _operators[client->GetNickName()] = client;
}

void Channel::RemoveOperator(Client* client) {
    _operators.erase(client->GetNickName());
}

const std::map<std::string, Client*>& Channel::GetMembers() const {
    return _members;
}

void Channel::InviteMember(std::string nick) {
    _invited_members.insert(nick);
}

void Channel::UninviteMember(std::string nick) {
    _invited_members.erase(nick);
}

bool Channel::IsInvited(std::string nick) const {
    return _invited_members.find(nick)!= _invited_members.end();
}

ChannelModes& Channel::GetModes() {
    return _modes;
}

const ChannelModes& Channel::GetModes() const {
    return _modes;
}

void Channel::Broadcast(const std::string& message, Client* sender) {
    for (std::map<std::string, Client*>::iterator it = _members.begin(); it != _members.end(); ++it) {
        if (it->second != sender) {
            it->second->GetOutBuffer().append(message + "\r\n");
            it->second->SetPollOut(true);
        }
    }
}
int Channel::GetClientCount() {
    return _members.size();
}
time_t Channel::GetCreationTime()
{
    return _creationTime;
}
void Channel::setCreationTime(time_t time)
{
    _creationTime = time;
}
void Channel::setTopicSetter(const std::string& setter) 
{
    _topicSetter = setter;
}

const std::string& Channel::getTopicSetter() const 
{
    return _topicSetter;
}

void Channel::setTopicSetTime(const std::string& time) 
{
    _topicSetTime = time;
}

const std::string& Channel::getTopicSetTime() const 
{
    return _topicSetTime;
}