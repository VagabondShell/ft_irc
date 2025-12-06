#include "../../includes/Server.hpp"

std::vector<std::string> generateElements(std::string str)
{
    std::vector<std::string> elmnts;
    std::string elm = "";
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == ' ')
            break;
        if (str[i] == ',')
        {
            if(elm.find(' ') == std::string::npos)
                elmnts.push_back(elm);
            elm = "";
        }
        else
            elm += str[i];
    }
    if((elm.find(' ') == std::string::npos && !elm.empty()))
            elmnts.push_back(elm);

    return elmnts;
}
std::string channel_members(Channel const &chan)
{
    const std::map<std::string, Client*>& members = chan.GetMembers();

    std::map<std::string, Client*>::const_iterator it;
    std::string list = "";
    for (it = members.begin(); it != members.end(); ++it)
    {
        Client* c = it->second;
        std::cout<< it->first<<std::endl;
        if (chan.IsMember(c))
        {
            if (chan.IsOperator(c))
            {
                list += "@" + c->GetNickName() + " ";
            }
            else
                list += c->GetNickName() + " ";
        }
    }
    return list;
}
void respone_msg(Client *client,std::string prefix,std::string channel_name,Channel *channel)
{

    if(client) 
    {
        client->GetOutBuffer().append((prefix+" JOIN " + channel_name + "\r\n"));
        client->SetPollOut(true);
        if(!channel->GetTopic().empty())
        {
            client->SendReply("332",channel_name+ " " + channel->GetTopic());
            client->SendReply("333",channel_name+" "+ channel->getTopicSetter() +" " + channel->getTopicSetTime());
        }
        client->SendReply("353","= " + channel_name+" :" + channel_members(*channel));
        client->SendReply("366",channel_name+" :End of /NAMES list.");
        channel->Broadcast((prefix + " JOIN " + channel_name),client);
    }
}
bool check_channel(std::string channel)
{
    if (channel.empty() || channel.size() > 200 ||
        (channel[0] != '#' && channel[0] != '&') ||
        channel.find('\t') != std::string::npos)
        return false;
    return true;
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
    std::string prefix = ":" + client->GetNickName() + "!" + client->GetUserName() +
                          "@" + client->GetIpAddress();
    std::vector<std::string>::iterator it;
    
    std::map<std::string,Channel *>::iterator channel_it;
    it = args.begin() + 1;
    channels = generateElements(*it);
   
    it++;
    if (it != args.end())
        keys = generateElements(*it);
    for (size_t i = 0; i < channels.size(); i++)
    {
        if(channels[i].size() == 1 && channels[i] == "0")
        {
            const std::set<Channel*> channels_set = client->GetClientChannels();
            std::set<Channel*>::iterator set_it;
            std::vector<std::string> args_part;
            std::string channels_names;
           
            for ( set_it = channels_set.begin(); set_it != channels_set.end(); set_it++)
            {
                Channel *chan = *set_it;
                channels_names += chan->GetName()+",";
            }
            if(channels_names.empty())
                continue;
            args_part.push_back("PART");
            args_part.push_back(channels_names);
            handlePartCommand(client,args_part);
            continue;
        }
        if (!check_channel(channels[i]))
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
                channel_obj->setCreationTime(time(NULL));
                _channels[channels[i]] = channel_obj;
                client->addChannel(channel_obj);
                respone_msg(client,prefix,channels[i],channel_obj);
            }
            else
            {
                ChannelModes Mods = channel_it->second->GetModes();
                if(channel_it->second->IsMember(client))
                    continue;
                if(channel_it->second->IsInvited(client->GetNickName()))
                {
                    channel_it->second->AddMember(client);
                    channel_it->second->UninviteMember(client->GetNickName());
                    client->removeInvitedchannel(channel_it->second);
                    client->addChannel(channel_it->second);
                    respone_msg(client,prefix,channels[i],channel_it->second);
                    continue;
                }
                if(Mods.inviteOnly && !channel_it->second->IsInvited(client->GetNickName()))
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
                if(Mods.userLimitSet && channel_it->second->GetMembers().size() >= Mods.userLimit)
                {   
                    client->SendReply("471",":Cannot join channel, Channel is full (+l)");
                    continue;
                }
                 channel_it->second->AddMember(client);
                 client->addChannel(channel_it->second);
                 channel_it->second->UninviteMember(client->GetNickName());
                 client->removeInvitedchannel(channel_it->second);
                 respone_msg(client,prefix,channels[i],channel_it->second);
            }
       }
    }
}
