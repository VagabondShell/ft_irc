#include "../includes/Server.hpp"

Server::Server(const int port, const std::string password)
    : _password(password), _port(port), _listenerFd(-1),
    _serverName("ft_irc.local")
{
    this->_commandMap["PASS"] = CMD_PASS;
    this->_commandMap["NICK"] = CMD_NICK;
    this->_commandMap["USER"] = CMD_USER;
    this->_commandMap["PRIVMSG"] = CMD_PRIVMSG;
    this->_commandMap["PONG"] = CMD_PONG;
    this->_commandMap["MODE"] = CMD_MODE;
    this->_commandMap["JOIN"] = CMD_JOIN;
    this->_commandMap["INVITE"] = CMD_INVITE;
    this->_commandMap["KICK"] = CMD_KICK;
    this->_commandMap["TOPIC"] = CMD_TOPIC;
    this->_commandMap["PART"] = CMD_PART;

    _listenerFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (_listenerFd < 0) {
        throw std::runtime_error("Socket creation failed.");
    }
    int opt_val = 1; 
    if (setsockopt(_listenerFd, SOL_SOCKET, SO_REUSEADDR, &opt_val,
                   sizeof(opt_val)) < 0) {
        close(_listenerFd);
        throw std::runtime_error("setsockopt failed.");
    }

    fcntl(_listenerFd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(_port);

    if (bind(_listenerFd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) == -1) {
        std::cerr << "Error: Failed to bind socket to port " << _port << "."
            << std::endl;
        close(_listenerFd);
        throw std::runtime_error(
            "Fatal server initialization error (bind failed).");
    }

    if (listen(_listenerFd, SOMAXCONN) == -1) {
        close(_listenerFd);
        throw std::runtime_error("Listen failed.");
    }

    struct pollfd listener_poll_fd = {};
    listener_poll_fd.fd = _listenerFd;
    listener_poll_fd.events = POLLIN;
    listener_poll_fd.revents = 0;
    _pollFds.push_back(listener_poll_fd);

    std::cerr << GREEN
              << "[SERVER START] Operational on port " << _port 
              << ". Waiting for connections..." << std::endl;
}

std::map<std::string, Client *> Server::GetNickNames() const{
    return _nicknames;
}

std::vector<struct pollfd> & Server::getPollfds(){
    return _pollFds;
}

e_cmd_type Server::getCommandType(std::string command) {
    
    std::map<std::string, e_cmd_type>::iterator it = _commandMap.find(command);

    if (it != _commandMap.end()) {
        return it->second;
    } else {
        return CMD_UNKNOWN; 
    }
}

bool Server::handleOutgoingData(int clientFd){

    Client* client = _clients[clientFd];
    std::string& buffer = client->GetOutBuffer();
    const char* data_ptr = buffer.c_str(); 
    size_t length = buffer.length();

    ssize_t bytes_sent = send(client->GetFd(), data_ptr, length, 0);
    if (bytes_sent > 0) {
        buffer.erase(0, bytes_sent);
        if (buffer.empty()) {
            client->SetPollOut(false); 
        }
    } else if (bytes_sent < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            return true;
        }
    }
    return false;
}

void Server::checkRegistration(Client * client){
    if (client->GetPassState() && client->GetUserState() && client->GetNickNameState())
    {
        std::string identity = client->GetNickName() + "!" + client->GetUserName() +
            "@" + client->GetIpAddress(); 
        std::string welcome_content = ":Welcome to the ft_irc.local Network, " + identity;
        client->SendReply("001", welcome_content);
        client->SetRegistration();
    }
}

void Server::handleNewConnection() {
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    std::memset(&client_addr, 0, addr_size);
    int new_client_fd =
        accept(_listenerFd, (struct sockaddr *)&client_addr, &addr_size);
    if (new_client_fd == -1) {
        perror("accept failed"); 
        return; 
    }
    if (fcntl(new_client_fd, F_SETFL, O_NONBLOCK) == -1) {
        perror("fcntl failed");
        close(new_client_fd); 
        return;
    }
    char* ip_temp_ptr = inet_ntoa(client_addr.sin_addr);
    std::string ip_str = (ip_temp_ptr) ? ip_temp_ptr : "0.0.0.0";
    _clients[new_client_fd] = new Client(new_client_fd, this);
    _clients[new_client_fd]->SetIpAddress(ip_str);
    
    //TODO understand it in more depth
    struct pollfd client_poll_fd = {};
    client_poll_fd.fd = new_client_fd;
    client_poll_fd.events = POLLIN;
    client_poll_fd.revents = 0;
    _pollFds.push_back(client_poll_fd);
}

bool Server::is_active(std::string nickname)
{
    return _nicknames.find(nickname) != _nicknames.end();
}
bool Server::handleClientCommand(const int current_fd) {

    Client *client = _clients[current_fd];

    // if (!client) {
    //     // Handle case where client object is somehow missing (fatal error)
    //     return;
    // }

    char temp_buffer[1024];
    ssize_t bytes_read =
        recv(current_fd, temp_buffer, sizeof(temp_buffer) - 1, 0);

    if (bytes_read == 0) {
        return true;

    } else if (bytes_read < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            return false; 
        } else {
            // perror("Fatal Recv Error");
            // perror("Fatal Recv Error");
            return true; 
        }
    }
    else {
        temp_buffer[bytes_read] = '\0';
        client->GetReadBuffer().append(temp_buffer, bytes_read);
        client->ProcessAndExtractCommands();
    return false;
    }
}

std::vector<std::string> split_string_to_vector(const std::string &input_string,
                                                char delimiter) {
    size_t colon_pos = input_string.find(':');
    std::string positional_part;
    std::string trailing_part;
    if (colon_pos == std::string::npos) {
        positional_part = input_string;
        trailing_part = "";
    } else {
        positional_part = input_string.substr(0, colon_pos);

        if (colon_pos + 1 < input_string.length()) {
            trailing_part = input_string.substr(colon_pos + 1);
        }
    }
    std::vector<std::string> tokens;
    std::stringstream ss(positional_part);
    std::string segment;
    while (std::getline(ss, segment, delimiter)) {
        if (!segment.empty())
            tokens.push_back(segment);
    }
    if (!trailing_part.empty() || (colon_pos != std::string::npos)){
        tokens.push_back(trailing_part);
    }
    return tokens;
}

void Server::commandDispatcher(Client *client, std::string commandLine) {
    std::vector<std::string> splitedCommand =
        split_string_to_vector(commandLine, ' ');
    if (splitedCommand.empty()) {
        return; 
    }
    std::string command = splitedCommand[0];
    e_cmd_type cmd = this->getCommandType(command);
    if (cmd == CMD_UNKNOWN){
            client->SendReply("421" , command + " :Unknown command");
            return;
    }
    if (cmd != CMD_PASS && cmd != CMD_NICK && cmd != CMD_USER) {
        if (!client->IsRegistered()) {
            client->SendReply("451",":You have not registered");
            return; 
        }
    }
    std::cout << "full cmd: " << commandLine << std::endl;
    switch (cmd) {
        case CMD_PASS:
            handlePassCommand(client, splitedCommand); 
            break;
        case CMD_NICK:
            handleNickCommand(client, splitedCommand);
            break;
        case CMD_USER:
            handleUserCommand(client, splitedCommand);
            break;
        case CMD_JOIN:
            handleJoinCommand(client, splitedCommand);
            break;
        case CMD_INVITE:
            handleInviteCommand(client, splitedCommand);
            break;
        case CMD_KICK:
            handleKickCommand(client, splitedCommand);
            break;
        case CMD_TOPIC:
            handleTopicCommand(client, splitedCommand);
            break;
        case CMD_PRIVMSG:
            handlePrivmsgCommand(client, splitedCommand);
            break;
        case CMD_MODE:
            handleModeCommand(client, splitedCommand);
            break;
        case CMD_PART:
            handlePartCommand(client, splitedCommand);
            break;
        default:
            break;
    }
}

void Server::disconnectClient(int current_fd) {
    Client* clientToDelete = _clients[current_fd];
    //TODO see if i need to protect this
    std::cerr << RED
        << "[DISCONNECT] Client disconnected."
        << " Nickname: " << (clientToDelete->GetNickName().empty() ? "(Unregistered)" 
        : clientToDelete->GetNickName()) << " | FD: " << current_fd << std::endl;
        clientToDelete->leftAllchannels();
    _nicknames.erase(clientToDelete->GetNickName());
   
    close(current_fd);
    delete clientToDelete;
    _clients.erase(current_fd);
}

time_t Server::getStartTime() const {
    return _StartTime;
}

void Server::run() {
    

    bool disconnected ;
    while (true) {
        std::cerr <<  GREEN <<"Polling on " << _pollFds.size() << " FDs."
            << std::endl;
        int ret = poll(&_pollFds[0], _pollFds.size(), -1);
        if (ret < 0) {
            throw std::runtime_error("Poll fatal error");
        }
        for (long unsigned int i = 0; i < _pollFds.size(); ++i) {
            disconnected = false;
            int current_fd = _pollFds[i].fd;
            if (_pollFds[i].revents & (POLLHUP | POLLERR)) {
                std::cerr << "[LOG] Abrupt disconnect detected on FD " << current_fd << std::endl;
                disconnected = true; 
            }
            else if (_pollFds[i].revents & POLLIN) {
                if (current_fd == _listenerFd)
                    handleNewConnection();
                else
                    disconnected = handleClientCommand(current_fd);
            }
            if (_pollFds[i].revents & POLLOUT){
                disconnected = handleOutgoingData(current_fd);

            }
            if (disconnected) {
                disconnectClient(current_fd); 
                
                _pollFds.erase(_pollFds.begin() + i); 
                i--; 
                continue;
            }

        }
    }
}

void Server::remove_channel(std::string channelName)
{
    
    _channels.erase(channelName);
}
Client *Server::GetClientByNick(std::string nick)
{
    std::map<std::string, Client*>::iterator it = _nicknames.find(nick);
    
    if (it != _nicknames.end())
        return it->second;
    return NULL; 
}
