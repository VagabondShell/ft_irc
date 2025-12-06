#include "../../includes/Server.hpp"
#include <cstdlib>
#include <sstream>


void create_modes(const std::vector<std::string>& args, std::vector<std::string>& modes, std::vector<std::string>& modeParams, Client* client) 
{
    if (args.size() < 3) 
        return;

    std::string modeStr = args[2];
    modes.clear();
    modeParams.clear();

    bool set_mode = true;
    size_t paramIndex = 3; 

    for (size_t i = 0; i < modeStr.size(); ++i) 
    {
        char c = modeStr[i];
        if (c == '+') 
        { 
            set_mode = true;
            continue; 
        }
        if (c == '-') 
        {
            set_mode = false; 
            continue; 
        }

        if (c != 'i' && c != 't' && c != 'k' && c != 'l' && c != 'o') 
        {
            client->SendReply("472", std::string(1, c) + " :is unknown mode char to me");
            continue;
        }

        std::string fullMode = (set_mode ? "+" : "-") + std::string(1, c);

        modes.push_back(fullMode);
    
        if (c == 'k' || c == 'l' || c == 'o') 
        {
            if (paramIndex < args.size()) 
                modeParams.push_back(args[paramIndex++]);
            else 
                modeParams.push_back("");
        }
    }
}


void Server::execute_modes(Client* client, Channel *channel, const std::vector<std::string>& modes, const std::vector<std::string>& modeParams) 
{
    if (!channel)
        return;
    size_t paramIndex = 0;

    
    int i = 0;
    for (i = 0; i < (int)modes.size(); ++i) 
    {
        std::string mode_str = modes[i];
        char c = mode_str[1];

        bool set_mode = true;
        if (mode_str[0] == '-') 
            set_mode = false;

        if (c == 'i') 
        {
            channel->GetModes().inviteOnly = set_mode;
            channel->Broadcast(":ft_irc.local MODE " + channel->GetName() + " " + mode_str, NULL);
        } 
        else if (c == 't') 
        {
            channel->GetModes().topicOpOnly = set_mode;
            channel->Broadcast(":ft_irc.local MODE " + channel->GetName() + " " + mode_str, NULL);

        } 
        else if (c == 'k') 
        {
            if (set_mode) 
            {
                if (paramIndex >= modeParams.size() || modeParams[paramIndex].empty()) 
                {
                    client->SendReply("696", channel->GetName() + " " + c +" :No parameter provided");
                    continue;
                }

                std::string new_pass = modeParams[paramIndex++];
                if (new_pass.find(" ") != std::string::npos)
                {
                    client->SendReply("525", channel->GetName() + " :Key is not well-formed");
                    continue;
                }
                channel->GetModes().passwordSet = true;
                channel->GetModes().password = new_pass;
                channel->Broadcast(":ft_irc.local MODE " + channel->GetName() + " " + mode_str + " " + new_pass, NULL);

            } 
            else 
            {
                channel->GetModes().passwordSet = false;
                channel->GetModes().password = "";
                channel->Broadcast(":ft_irc.local MODE " + channel->GetName() + " " + mode_str, NULL); 
            }
        } 
        else if (c == 'l') 
        {
            if (set_mode) 
            {
                if (paramIndex >= modeParams.size() || modeParams[paramIndex].empty()) 
                {
                    client->SendReply("696", channel->GetName() + " " + c +" :No parameter provided");
                    continue;
                }
 
                size_t limit = atoi(modeParams[paramIndex].c_str());
                if (limit <= 0) 
                {
                    client->SendReply("696", channel->GetName() + " " + c +" :Invalid limit parameter");
                    continue;
                }
                channel->GetModes().userLimitSet = true;
                channel->GetModes().userLimit = (size_t)limit;
                channel->Broadcast(":ft_irc.local MODE " + channel->GetName() + " " + mode_str + " " + modeParams[paramIndex].c_str(), NULL);
                paramIndex++;
            } 
            else 
            {
                channel->GetModes().userLimitSet = false;
                channel->GetModes().userLimit = 0;
                channel->Broadcast(":ft_irc.local MODE " + channel->GetName() + " " + mode_str, NULL); 
            }
        }
        else if (c == 'o')
        {
            if (paramIndex >= modeParams.size() || modeParams[paramIndex].empty())
            {
                client->SendReply("696", channel->GetName() + " " + c +" :No parameter provided");
                continue;
            }

            std::string tobe_operator = modeParams[paramIndex++];

            std::map<std::string, Client*>::iterator itClient = _nicknames.find(tobe_operator);

            if (itClient == _nicknames.end())
            {
                client->SendReply("401", tobe_operator + " :No such nick");
                continue;
            }

            Client* target = itClient->second;

            if (!channel->IsMember(target))
            {
                client->SendReply("441", tobe_operator + " " + channel->GetName() + " :They aren't on that channel");
                continue;
            }

            if (set_mode)
                channel->AddOperator(target);
            else
                channel->RemoveOperator(target);
    
            channel->Broadcast(":ft_irc.local MODE " + channel->GetName() + " " + mode_str + " " +  tobe_operator, NULL);         
        }
    }
}


void Server::handleModeCommand(Client *client, std::vector<std::string> args)
{
    if (args.size() < 2) {
        client->SendReply("461", "MODE :Not enough parameters");
        return;
    }

    std::string channelName = args[1];
    std::map<std::string, Channel*>::iterator it_ch = _channels.find(channelName);


    if (it_ch == _channels.end()) 
    {
        client->SendReply("403", channelName + " :No such channel");
        return;
    }
    Channel* channel = it_ch->second;
    bool is_op = true;
    if (!channel->IsOperator(client)) 
        is_op = false;

    if (args.size() == 2) {
        std::string modes = channel->GetModes().toString(is_op);
        client->SendReply("324", channelName + " " + modes);
        std::stringstream time_str;
        time_str << channel->GetCreationTime();
        client->SendReply("329", channelName + " " + time_str.str());
        return;
    }
    if (!is_op) {
        client->SendReply("482", channel->GetName() + " :You're not channel operator");
        return ;
    }
    
    std::string modes = args[2];

    std::vector<std::string> modes_vec;
    std::vector<std::string> parms;
    
    create_modes(args, modes_vec, parms, client);
    execute_modes(client, channel, modes_vec, parms);

}
