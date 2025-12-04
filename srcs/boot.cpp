#include "../includes/Server.hpp"
#include <unistd.h>

std::vector<std::string> splitVector(const std::string &input_string,
                                                char delimiter) {
    std::string positional_part;
    positional_part = input_string;
    std::vector<std::string> tokens;
    std::stringstream ss(positional_part);
    std::string segment;
    while (std::getline(ss, segment, delimiter)) {
        if (!segment.empty())
            tokens.push_back(segment);
    }
    return tokens;
}
// e_cmd_bot_type getCmdTtype(const std::string & cmdName)
// {
//     const std::string botCmdsNames[] = {
//         "!help",
//         "!time",
//         "!joke",
//     };
//     for (size_t i = 0; i < 4; i++) {
//         if (cmdName == botCmdsNames[i])
//             return e_cmd_bot_type(i);
//     }
//     return BOT_CMD_UNKNOWN;
// }
//
// void SendPrivateMessage(int bot_fd, std::string & SendClient, const std::string &Message){
//     std::string FullMessage =  "PRIVMSG " + SendClient + " :" + Message + "\r\n" ;
//     std::cout << "FullMessage: " << FullMessage << std::endl;
//     if (send(bot_fd, Message.c_str(), Message.length(), 0) == -1){
//         throw std::runtime_error("Bot connection failed during send.");
//     }
//
// }
//
// void handleBotJoke(int bot_fd, std::string & SendClient, const std::vector<std::string>& args) {
//     
//     if (args.size() != 1) { 
//         // Queue error: "!joke takes no arguments. Usage: !joke"
//         return; 
//     }
//     std::vector<std::string> jokes;
//     jokes.push_back("I’d tell them a UDP joke but there’s no guarantee that they would get it.");
//     jokes.push_back("Q. How do robots eat pizza? A. One byte at a time.");
//     jokes.push_back("Q. How did the first program die? A. It was executed.");
//     jokes.push_back("Q. Why do Java programmers wear glasses? A. They can’t C#");
//     jokes.push_back("Q. What is the difference between C++ and C? A. Just 1.");
//     jokes.push_back("Q. Whats the object-oriented way to become wealthy? A. Inheritance");
//     jokes.push_back("Q. What do computers and air conditioners have in common? A. They both become useless when you open windows.");
//     size_t joke_count = jokes.size();
//     int random_index = rand() % joke_count; 
//     std::string selected_joke = jokes[random_index];
//     SendPrivateMessage(bot_fd, SendClient, selected_joke);
// }
//
// void HelpCmd(int bot_fd, std::string & Sendclient, std::vector<std::string>& arguments){
//     std::string message;
//     std::vector<std::string> HelpMessages;
//     HelpMessages.push_back("-- FTBot Command List --");
//     HelpMessages.push_back("help :Displays this command list.");
//     HelpMessages.push_back("time :Displays the current server time and date.");
//     HelpMessages.push_back("------------------------");
//     std::cout << "here" << std::endl;
//     for (size_t i = 0; i < HelpMessages.size(); i++)
//          SendPrivateMessage(bot_fd, Sendclient, HelpMessages[i]);
// }
//
// void timeCmd(int bot_fd, std::string & Sendclient, std::vector<std::string>& arguments){
//      std::string message;
//      time_t RawSeconds;
//      time(&RawSeconds);
//      struct tm *TimeInfo = localtime(&RawSeconds);
//      char TimeBuffer[100];
//      size_t len = strftime(TimeBuffer, sizeof(TimeBuffer), 
//                            "Current server time: %H:%M:%S on %Y-%m-%d", 
//                            TimeInfo);
//      if (len > 0)
//          SendPrivateMessage(bot_fd, Sendclient, TimeBuffer);
// }

void processBotCommand(int bot_fd, std::string & FullMessage){
    (void)bot_fd;
    std::vector<std::string> splitedCommand =
        splitVector(FullMessage, ' ');

    for (size_t i = 0; i < splitedCommand.size(); i++){
        std::cout << "splitedCommand: " << i << " = " << splitedCommand[i] << std::endl; 

    }
    if (splitedCommand.empty()) {
        return; 
    }
    if (splitedCommand.size() < 4) {
        std::string message = "Syntax Error: Need more args arguments for the Bot command. Use help for command list.";
        return; 
    }
    else if (splitedCommand.size() > 4)
    {
        std::string message = "Syntax Error: Too many arguments for the Bot command. Use help for command list.";
        // SendPrivateMessage(bot_fd, splitedCommand[0], message);
        return;
    }
    std::string Sendclient = splitedCommand[0];
    std::string recivedCommand = splitedCommand[1];
    
    std::cout << "Name of the client " << Sendclient << std::endl;
    std::cout << "Command" << recivedCommand << std::endl;
    return;
    // e_cmd_bot_type cmdType = getCmdTtype(splitedCommand[1]) ;
    // switch (cmdType) {
    //     case BOT_CMD_HELP:
    //         HelpCmd(bot_fd, Sendclient, splitedCommand );
    //         break;
    //     case BOT_CMD_TIME:
    //         timeCmd(bot_fd, Sendclient, splitedCommand );
    //         break;
    //     case BOT_CMD_JOKE:
    //         handleBotJoke(bot_fd, Sendclient, splitedCommand );
    //         break;
    //     case BOT_CMD_UNKNOWN:
    //         std::string message = splitedCommand[1] + "is not a recognized command. Use help for a list of available commands.";
    //         SendPrivateMessage(bot_fd, Sendclient, message);
    //         break;
    //     // ...
    // }
}

int setup_connection(const std::string& server_ip, int port) {
    int bot_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    if (bot_socket_fd < 0) {
        throw std::runtime_error("Failed to create socket.");
    }

    struct sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 
    serv_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    if (connect(bot_socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(bot_socket_fd);
        throw std::runtime_error("Connection refused or failed.");
    }
    
    return bot_socket_fd;
}

void SendMessage(int bot_fd, const std::string &Message){
    if (send(bot_fd, Message.c_str(), Message.length(), 0) == -1){
        throw std::runtime_error("Bot connection failed during send.");
    }
}

std::string ExtractAndEraseFromBuffer(size_t pos_found, int dilimiterLen, std::string &_ReadBuffer) {
  std::string toRetrun = _ReadBuffer.substr(0, pos_found);
  _ReadBuffer = _ReadBuffer.substr(pos_found + dilimiterLen);
  return toRetrun;
}

void ProcessAndExtractCommands(int bot_fd, std::string &_ReadBuffer) {
  int dilimiterLen;
  size_t pos_found = _ReadBuffer.find("\r\n");
  dilimiterLen = 2;
  if (pos_found == std::string::npos)
  {
    pos_found = _ReadBuffer.find("\n");
    dilimiterLen = 1;
  }

  while (pos_found != std::string::npos) {
    std::string command_line = ExtractAndEraseFromBuffer(pos_found, dilimiterLen, _ReadBuffer);
    processBotCommand(bot_fd, command_line);
    pos_found = _ReadBuffer.find("\r\n");
    if (pos_found == std::string::npos)
      pos_found = _ReadBuffer.find("\n");
  }
}

void start_bot_loop(int bot_fd, const std::string& password, const std::string& nick_name) {
    
    std::string bot_nick = nick_name;
    std::string bot_user = "BotUser";
    SendMessage(bot_fd, "PASS " + password + "\r\n");
    SendMessage(bot_fd, "NICK " + bot_nick + "\r\n");
    SendMessage(bot_fd, "USER " + bot_user + " 0 * :" + bot_user + " Service Bot\r\n");
    std::string bot_in_buffer;
    while (true) {

        int bytes_read = 0;
	    char buff[1025];
	    bytes_read = recv(bot_fd, buff, 1024, 0);
	    if (bytes_read == 0){
	        close(bot_fd);
	        return;
	    }else {
            buff[bytes_read] = '\0';
            bot_in_buffer.append(buff, bytes_read);
            ProcessAndExtractCommands(bot_fd, bot_in_buffer);
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << "<nick_name> <server_ip> <port> <password>" << std::endl;
        return 1;
    }

    int bot_fd = -1;
    std::string nick_name = argv[1];
    std::string server_ip = argv[2];
    int port = std::atoi(argv[3]); 
    std::string password = argv[4];

    if (port <= 1024 || port > 65535) {
        std::cerr << "Error: Invalid port number." << std::endl;
        return 1;
    }
    
    try {
        bot_fd = setup_connection(server_ip, port);
        start_bot_loop(bot_fd, password, nick_name);

    } catch (const std::exception &e) {
        std::cerr << "Bot Fatal Error: " << e.what() << std::endl;
        if (bot_fd > 0)
            close(bot_fd);
        return 1;
    }
    
    return 0;
}
