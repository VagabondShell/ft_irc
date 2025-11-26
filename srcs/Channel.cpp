#include "Channel.hpp"

Channel::Channel()
{
    
}
Channel::Channel(std::string name,std::string key,std::string topic)
{
    this->name = name;
    this->key = key;
    this->topic = topic;
    this->isinvite_only = 0;
}
void Channel::kick_client(Client *client)
{
    for (Client *c:clients)
    {
        if(c)
    }
    
}