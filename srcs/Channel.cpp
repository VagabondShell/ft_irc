#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel(const std::string& name) : _name(name), _topic("") {}
bool Channel::GetInvite()const
{
    return _is_invite_only;
}

const std::string& Channel::GetName() const {
    return _name;
}

const std::string& Channel::GetTopic() const {
    return _topic;
}

bool Channel::IsMember(Client* client) const {
    return _members.find(client) != _members.end();
}

bool Channel::IsOperator(Client* client) const {
    return _operators.find(client) != _operators.end();
}

void Channel::AddMember(Client* client) {
    _members.insert(client);
}

void Channel::RemoveMember(Client* client) {
    _members.erase(client);
    _operators.erase(client); 
}

void Channel::AddOperator(Client* client) {
    if (IsMember(client))
        _operators.insert(client);
}

void Channel::RemoveOperator(Client* client) {
    _operators.erase(client);
}

void Channel::SetTopic(const std::string& topic) {
    _topic = topic;
}

void Channel::Broadcast(const std::string& message, Client* sender) {
    for (std::set<Client*>::iterator it = _members.begin(); it != _members.end(); ++it) {
        if (*it != sender) {
            (*it)->GetOutBuffer().append(message + "\r\n");
            (*it)->SetPollOut(true);
        }
    }
}

std::vector<Client*> Channel::GetMembers() const {
    return std::vector<Client*>(_members.begin(), _members.end());
}
