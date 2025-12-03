#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <vector>
#include <sstream>

#include <set>
class Client;

struct ChannelModes {
    bool inviteOnly;        // +i
    bool topicOpOnly;       // +t
    bool passwordSet;       // +k
    std::string password;
    bool userLimitSet;      // +l
    size_t userLimit;

    ChannelModes()
        : inviteOnly(false),
          topicOpOnly(true),
          passwordSet(false),
          userLimitSet(false),
          userLimit(0)
    {}

    std::string toString() const
    {
        std::string flags;
        std::string params;
        if (inviteOnly) flags += "i";
        if (topicOpOnly) flags += "t";
        if (passwordSet) { flags += "k"; params += "password_Hided"; }
        if (userLimitSet) {
            flags += "l";
            std::ostringstream oss;
            oss << userLimit;
            params += " " + oss.str();
        }
        if (flags.empty()) return "";
        return std::string("+") + flags + params;
    }
};

class Channel {
public:
    Channel(const std::string& name);

    const std::string& GetName() const;
    const std::string& GetTopic() const;
    void SetTopic(const std::string& topic);

    bool IsMember(Client* client) const;
    bool IsMemberByNick(std::string nick) const;
    void RemoveMemberByNick(std::string nick);
    bool IsOperator(Client* client) const;
    void AddMember(Client* client);
    void RemoveMember(Client* client);
    void AddOperator(Client* client);
    void RemoveOperator(Client* client);
    int GetClientCount();
    time_t GetCreationTime();
    const std::map<std::string, Client*>& GetMembers() const;
    void InviteMember(std::string nick);
    void UninviteMember(std::string nick);
    bool IsInvited(std::string nick) const;
    ChannelModes& GetModes();
     
    const ChannelModes& GetModes() const;
    void Broadcast(const std::string& message, Client* sender = NULL);
    
    void setTopicSetter(const std::string& setter);
    void setCreationTime(time_t time);
    const std::string& getTopicSetter() const;
    void setTopicSetTime(const std::string& time);
    const std::string& getTopicSetTime() const;
    
private:
    std::string _name;
    std::string _topic;
    std::map<std::string, Client*> _members;
    std::map<std::string, Client*> _operators;
    std::set<std::string> _invited_members;
    ChannelModes _modes;
    time_t      _creationTime;
    std::string _topicSetter;
    std::string _topicSetTime;    
};

#endif