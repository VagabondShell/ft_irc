#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <set>
#include <vector>
#include "Client.hpp"

class Channel {
    public:
        Channel(const std::string& name);

        const std::string& GetName() const;
        const std::string& GetTopic() const;

        bool IsMember(Client* client) const;
        bool IsOperator(Client* client) const;

        void AddMember(Client* client);
        void RemoveMember(Client* client);
        void AddOperator(Client* client);
        void RemoveOperator(Client* client);

        void SetTopic(const std::string& topic);

        void InviteMember(Client* client);
        void UninviteMember(Client* client);
        bool IsInvited(Client* client) const;

        void Broadcast(const std::string& message, Client* sender = NULL);
        bool GetInvite() const;
        void SetInvite(bool fact);
        std::vector<Client*> GetMembers() const;

    private:
        std::string _name;
        bool _is_invite_only;
        std::string _topic;
        std::set<Client*> _invited_members;
        std::set<Client*> _members;
        std::set<Client*> _operators;
};

#endif
