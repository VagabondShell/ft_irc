#include "../../includes/Server.hpp"

std::vector<std::string> genrateNames_keys(std::string str)
{
    std::vector<std::string> elmnts;
    std::string elm = "";
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == ' ')
            break;
        if (str[i] == ',')
        {
            elmnts.push_back(elm);
            elm = "";
        }
        else
            elm += str[i];
    }
    elmnts.push_back(elm);
    return elmnts;
}
bool check_channel(std::string channel)
{
    if (channel.empty() || channel.size() > 200 ||
        (channel[0] != '#' && channel[0] != '&') ||
        channel.find('\t') != std::string::npos)
        return true;
    return false;
}
void Server::handleJoinCommand(Client *client, std::vector<std::string> args)
{

    std::string content;
    Channel *channel_obj;
    if (args.size() < 2)
    {
        content = ":Not enough parameter";
        client->SendReply("461", content);
        return;
    }
    std::vector<std::string> channels;
    std::vector<std::string> keys;

    std::vector<std::string>::iterator it;
     std::map<std::string,Channel *>::iterator channel_it;
    it = args.begin() + 1;
    channels = genrateNames_keys(*it);
    it++;
    if (it != args.end())
        keys = genrateNames_keys(*it);
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (check_channel(channels[i]))
        {
            content = channels[i] +" :No such channel";
            client->SendReply("403", content);
        }
       else
       {
            channel_it = _channels.find(channels[i]);
            if(channel_it == _channels.end())
            {
                channel_obj = new Channel(channels[i]);
                channel_obj->AddMember(client);
                channel_obj->AddOperator(client);
                _channels[channels[i]] = channel_obj;
                client->SendReply("353","= "+channels[i]+" :"+client->GetNickName()+" " +this->_serverName);
            }
            else
            {
                ChannelModes Mods = channel_it->second->GetModes();
                if(channel_it->second->IsMember(client))
                    continue;
                if(channel_it->second->IsInvited(client))
                {
                    channel_it->second->AddMember(client);
                    client->SendReply("353","= "+channels[i]+" :"+client->GetNickName()+" " +this->_serverName);
                    continue;
                }
                if(Mods.inviteOnly && !channel_it->second->IsInvited(client))
                {
                     client->SendReply("473",channels[i]+" :Cannot join channel, you must be invited (+i)");
                    continue;
                }
                if(Mods.passwordSet)
                {
                    if(i >= keys.size() || Mods.password != keys[i])
                    {
                        client->SendReply("475",channels[i]+" :Cannot join channel, you need the correct key (+k)");
                        continue;
                    }
                }
                if(Mods.userLimitSet && _channels.size() >= Mods.userLimitSet)
                {
                    client->SendReply("471",":Cannot join channel, Channel is full (+l)");
                    continue;
                }
                 channel_it->second->AddMember(client);
                 client->SendReply("353","= "+channels[i]+" :"+client->GetNickName() +" join" +" " +this->_serverName);
            }
       }
    }
}
