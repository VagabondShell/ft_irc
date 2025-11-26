#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <vector>

class Client;

struct ChannelModes {
    bool inviteOnly;        // +i
    bool topicOpOnly;       // +t
    bool passwordSet;       // +k
    std::string password;
    bool userLimitSet;      // +l
    int userLimit;

    ChannelModes()
        : inviteOnly(false),
          topicOpOnly(true),
          passwordSet(false),
          userLimitSet(false),
          userLimit(0)
    {}
};

class Channel {
public:
    Channel(const std::string& name);


    const std::string& GetName() const;
    const std::string& GetTopic() const;
    void SetTopic(const std::string& topic);

    bool IsMember(Client* client) const;
    bool IsOperator(Client* client) const;
    void AddMember(Client* client);
    void RemoveMember(Client* client);
    void AddOperator(Client* client);
    void RemoveOperator(Client* client);
    int GetClientCount();
    const std::set<Client*>& GetMembers() const;
    void InviteMember(Client* client);
    void UninviteMember(Client* client);
    bool IsInvited(Client* client) const;
    ChannelModes& GetModes();
    const ChannelModes& GetModes() const;
    void Broadcast(const std::string& message, Client* sender = NULL);
private:
    std::string _name;
    std::string _topic;
    std::set<Client*> _members;
    std::set<Client*> _operators;
    std::set<Client*> _invited_members;
    ChannelModes _modes;
};

#endif
