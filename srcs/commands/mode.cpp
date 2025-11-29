#include "../../includes/Server.hpp"



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


bool Server::execute_modes(Client* client, const std::string& channelName, const std::vector<std::string>& modes, const std::vector<std::string>& modeParams) 
{
    Channel* channel = _channels[channelName];
    if (!channel)
        return false;
    size_t paramIndex = 0;


    if (!channel->IsOperator(client)) {
        client->SendReply("482", channelName + " :You're not channel operator");
        return false;
    }

    for (size_t i = 0; i < modes.size(); ++i) 
    {
        std::string mode_str = modes[i];
        char c = mode_str[1];

        bool set_mode = true;
        if (mode_str[0] == '-') 
            set_mode = false;

        if (c == 'i') 
        {
            channel->GetModes().inviteOnly = set_mode;
        } 
        else if (c == 't') 
        {
            channel->GetModes().topicOpOnly = set_mode;
        } 
        else if (c == 'k') 
        {
            if (set_mode) 
            {
                if (modeParams[paramIndex].empty()) 
                {
                    client->SendReply("461", "MODE +k :Not enough parameters");
                    return false;
                }
                std::string new_pass = modeParams[paramIndex++];
                channel->GetModes().passwordSet = true;
                channel->GetModes().password = new_pass;
            } 
            else 
            {
                channel->GetModes().passwordSet = false;
                channel->GetModes().password = "";
            }
        } 
        else if (c == 'l') 
        {
            if (set_mode) 
            {
                if (modeParams[paramIndex].empty()) 
                {
                    client->SendReply("461", "MODE +l :Not enough parameters");
                    return false;
                }

                // size_t limit = std::stoull(modeParams[paramIndex++]);
                
                size_t limit = atoi(modeParams[paramIndex++].c_str());
                if (limit <= 0) 
                {
                    client->SendReply("461", "MODE +l :Invalid limit parameter");
                    return false;
                }
                channel->GetModes().userLimitSet = true;
                channel->GetModes().userLimit = limit;
            } 
            else 
            {
                channel->GetModes().userLimitSet = false;
                channel->GetModes().userLimit = 0;
            }
        }
        else if (c == 'o')
        {
            if (modeParams[paramIndex].empty())
            {
                client->SendReply("461", "MODE +o/-o :Not enough parameters");
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
                client->SendReply("441", tobe_operator + " " + channelName + " :They aren’t on that channel");
                continue;
            }

            if (set_mode)
                channel->AddOperator(target);
            else
                channel->RemoveOperator(target);
        }
    }
    return true;
}


std::string filterValidModes(const std::string& modeStr) {
    std::string result;
    for (size_t i = 0; i < modeStr.size(); ++i) {
        char c = modeStr[i];
        if (c == '+' || c == '-' || c == 'i' || c == 'k' || c == 'l' || c == 't' || c == 'o') {
            result += c;
        }
    }
    return result;
}

void Server::handleModeCommand(Client *client, std::vector<std::string> args)
{
    if (args.size() < 2) {
        client->SendReply("461", "MODE :Not enough parameters");
        return;
    }
    if (args.size() == 2) {
        std::string channelName = args[1];
        if (_channels.find(channelName) == _channels.end()) {
            client->SendReply("403", channelName + " :No such channel");
            return;
        }
        Channel* channel = _channels[channelName];
        std::string modes = channel->GetModes().toString();
        client->SendReply("324", channelName + " " + modes);
        return;
    }
    std::string target = args[1];
    std::string modes = args[2];


    std::vector<std::string> modes_vec;
    std::vector<std::string> parms;

    if (!target.empty() && target[0] == '#')
    {
        if (_channels.find(target) == _channels.end()) 
        {
            client->SendReply("403", target + " :No such channel");
            return;
        }

    }
    else 
    {
        client->SendReply("403", target + " :No such channel");
        return;
    }
    
    create_modes(args, modes_vec, parms, client);
    if (execute_modes(client, target, modes_vec, parms))
    {
        std::string notify = ":ft_irc.local MODE " + target + " " + filterValidModes(modes);
        _channels[target]->Broadcast(notify, NULL);
    }   
}





