#include <iostream>     
#include <string>       
#include <vector>       
#include <sstream>      
#include <cstring>      
#include <cstdlib>      
#include <ctime>        
#include <stdexcept>    
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>  
#include <unistd.h>     
#include <fcntl.h>      
#include <cerrno>       
#define GREEN    "\x1b[32m" 

enum e_cmd_bot_type {
    BOT_CMD_HELP    = 0,
    BOT_CMD_TIME    = 1,
    BOT_CMD_JOKE  = 2, 
    BOT_CMD_UNKNOWN = 4,
};

std::vector<std::string> SplitVector(const std::string &InputString, 
                                     char Delimiter) {
    std::string PositionalPart;
    PositionalPart = InputString;
    std::vector<std::string> Tokens;
    std::stringstream StringStream(PositionalPart);
    std::string Segment;
    while (std::getline(StringStream, Segment, Delimiter)) {
        if (!Segment.empty())
            Tokens.push_back(Segment);
    }
    return Tokens;
}

e_cmd_bot_type GetCmdType(const std::string & CmdName)
{
    const std::string BotCmdsNames[] = {
        "!help",
        "!time",
        "!joke",
    };
    for (size_t i = 0; i < 3; i++) {
        if (CmdName == BotCmdsNames[i])
            return e_cmd_bot_type(i);
    }
    return BOT_CMD_UNKNOWN;
}

void SendPrivateMessage(int BotFd, std::string & SendClient, const std::string &Message){
    std::string FullMessage = "PRIVMSG " + SendClient + " :" + Message + "\r\n";
    if (send(BotFd, FullMessage.c_str(), FullMessage.length(), 0) == -1){
        throw std::runtime_error("Bot connection failed during send.");
    }
}


void HandleBotJoke(int BotFd, std::string &SendClient) {
    std::vector<std::string> Jokes;
    Jokes.push_back("I’d tell them a UDP joke but there’s no guarantee that they would get it.");
    Jokes.push_back("Q. How do robots eat pizza? A. One byte at a time.");
    Jokes.push_back("Q. How did the first program die? A. It was executed.");
    Jokes.push_back("Q. Why do Java programmers wear glasses? A. They can’t C#");
    Jokes.push_back("Q. What is the difference between C++ and C? A. Just 1.");
    Jokes.push_back("Q. What's the object-oriented way to become wealthy? A. Inheritance");
    Jokes.push_back("Q. What do computers and air conditioners have in common? A. They both become useless when you open windows.");
    
    size_t JokeCount = Jokes.size();
    int RandomIndex = rand() % JokeCount; 
    std::string SelectedJoke = Jokes[RandomIndex];
    SendPrivateMessage(BotFd, SendClient, SelectedJoke);
}

void HelpCmd(int BotFd, std::string & Sendclient){
    std::string message;
    std::vector<std::string> HelpMessages;
    HelpMessages.push_back("-- FTBot Command List --");
    HelpMessages.push_back("!help :Displays this command list.");
    HelpMessages.push_back("!time :Displays the current server time and date.");
    HelpMessages.push_back("!joke :Tells a random programming joke.");
    HelpMessages.push_back("------------------------");
    for (size_t i = 0; i < HelpMessages.size(); i++)
         SendPrivateMessage(BotFd, Sendclient, HelpMessages[i]);
}

void TimeCmd(int BotFd, std::string &SendClient) {
    time_t RawSeconds;
    time(&RawSeconds);
    struct tm *TimeInfo = localtime(&RawSeconds);
    char TimeBuffer[100];
    size_t Len = strftime(TimeBuffer, sizeof(TimeBuffer), 
                           "Current server time: %H:%M:%S on %Y-%m-%d", 
                           TimeInfo);
    if (Len > 0)
        SendPrivateMessage(BotFd, SendClient, TimeBuffer);
}

std::string ExtractNick(const std::string& Prefix) {
    size_t ExclamationPos = Prefix.find('!');
    if (ExclamationPos != std::string::npos) {
        return Prefix.substr(1, ExclamationPos - 1);
    }
    if (!Prefix.empty() && Prefix[0] == ':')
        return Prefix.substr(1);
        
    return Prefix;
}

std::string GetCmd(std::string Msg) {
    if (Msg.empty()) return "";

    if (Msg[0] == ':') {
        Msg = Msg.substr(1);
    }
    return Msg;
}

void ProcessBotCommand(int BotFd, std::string &FullMessage) {
    std::vector<std::string> SplitCommand = SplitVector(FullMessage, ' ');

    if (SplitCommand.empty()) {
        return; 
    }

    if (SplitCommand.size() < 3 || (SplitCommand.size() > 2 && SplitCommand[3].empty())) {
        std::string Message = "Syntax Error: Need more arguments for the Bot command. Use help for command list.";
        SendPrivateMessage(BotFd, SplitCommand[0], Message);
        return; 
    }
    else if (SplitCommand.size() > 4) {
        std::string Message = "Syntax Error: Too many arguments for the Bot command. Use help for command list.";
        SendPrivateMessage(BotFd, SplitCommand[0], Message);
        return;
    }

    std::string Prefix = SplitCommand[0];    
    std::string Msg = SplitCommand[3];
    std::string SendClient = ExtractNick(Prefix);
    std::string Cmd = GetCmd(Msg);
    
    e_cmd_bot_type CmdType = GetCmdType(Cmd); 
    
    switch (CmdType) {
        case BOT_CMD_HELP:
            HelpCmd(BotFd, SendClient);
            break;
        case BOT_CMD_TIME:
            TimeCmd(BotFd, SendClient);
            break;
        case BOT_CMD_JOKE:
            HandleBotJoke(BotFd, SendClient);
            break;
        case BOT_CMD_UNKNOWN:
            {
                std::string Message = Cmd + " is not a recognized command. Use !help for a list of available commands.";
                SendPrivateMessage(BotFd, SendClient, Message);
            }
            break;
    }
}

void SendMessage(int BotFd, const std::string &Message) {
    if (send(BotFd, Message.c_str(), Message.length(), 0) == -1) {
        throw std::runtime_error("Bot connection failed during send.");
    }
}

std::string ExtractAndEraseFromBuffer(size_t PosFound, int DelimiterLen, std::string &ReadBuffer) {
  std::string ToReturn = ReadBuffer.substr(0, PosFound);
  ReadBuffer = ReadBuffer.substr(PosFound + DelimiterLen);
  return ToReturn;
}

std::string GetServerAuth(int BotSocketFd) {
    std::string BotInBuffer = "";
    char Buff[1025];
    ssize_t BytesRead;

    while (BotInBuffer.find("\r\n") == std::string::npos) {
        std::memset(Buff, 0, 1025);
        BytesRead = recv(BotSocketFd, Buff, 1024, 0);
        if (BytesRead <= 0) {
            if (BytesRead == 0)
                throw std::runtime_error("Server closed connection unexpectedly.");
            else if (BytesRead < 0) {
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                    throw std::runtime_error("recv error.");
            }
        }
        if (BytesRead > 0)
            BotInBuffer.append(Buff, BytesRead);
    }
    
    int DelimiterLen;
    size_t PosFound = BotInBuffer.find("\r\n");
    DelimiterLen = 2;
    
    if (PosFound == std::string::npos) {
        PosFound = BotInBuffer.find("\n");
        DelimiterLen = 1;
    }
    
    std::string ToReturn = ExtractAndEraseFromBuffer(PosFound, DelimiterLen, BotInBuffer);
    return ToReturn;
}

int SetupConnection(const std::string& ServerIp, int Port, const std::string& Password, const std::string& NickName) {

    int BotSocketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    if (BotSocketFd < 0) {
        throw std::runtime_error("Failed to create socket.");
    }

    struct sockaddr_in ServAddr;
    std::memset(&ServAddr, 0, sizeof(ServAddr));
    ServAddr.sin_family = AF_INET; 
    ServAddr.sin_port = htons(Port); 
    ServAddr.sin_addr.s_addr = inet_addr(ServerIp.c_str());

    if (connect(BotSocketFd, (struct sockaddr *)&ServAddr, sizeof(ServAddr)) < 0) {
        close(BotSocketFd);
        throw std::runtime_error("Connection refused or failed.");
    }
    if (fcntl(BotSocketFd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(BotSocketFd); 
        throw std::runtime_error("fcntl failed.");
    }
    
    std::string BotUser = "BotUser";
    SendMessage(BotSocketFd, "PASS " + Password + "\r\n");
    SendMessage(BotSocketFd, "NICK " + NickName + "\r\n");
    SendMessage(BotSocketFd, "USER " + BotUser + " 0 * :" + BotUser + " Service Bot\r\n");
    
    std::string AuthResp = GetServerAuth(BotSocketFd);
    
    if (AuthResp.find(" 001 ") != std::string::npos) {
        std::cout << GREEN << "Bot successfully authenticated!" << std::endl;
        return BotSocketFd; 
    }
    
    close(BotSocketFd);
    if (AuthResp.find(" 433 ") != std::string::npos) {
        throw std::runtime_error("Bot connection failed: Nickname already in use (433).");
    }
    else if (AuthResp.find(" 464 ") != std::string::npos) {
        throw std::runtime_error("Bot connection failed: Password incorrect (464).");
    }
    else {
        throw std::runtime_error("Bot connection failed: Unknown server response.");
    }
}

void ProcessAndExtractCommands(int BotFd, std::string &ReadBuffer) {
  int DelimiterLen;
  size_t PosFound = ReadBuffer.find("\r\n");
  DelimiterLen = 2;

  while (PosFound != std::string::npos) {
    std::string CommandLine = ExtractAndEraseFromBuffer(PosFound, DelimiterLen, ReadBuffer);
    ProcessBotCommand(BotFd, CommandLine);
    PosFound = ReadBuffer.find("\r\n");
  }
}

void StartBotLoop(int BotFd) {
    std::string BotInBuffer;
    while (true) {
        int BytesRead = 0;
        char Buff[1025];
        BytesRead = recv(BotFd, Buff, 1024, 0);
        
        if (BytesRead == 0) {
            close(BotFd);
            return;
        } else if (BytesRead > 0) {
            Buff[BytesRead] = '\0';
            BotInBuffer.append(Buff, BytesRead);
            ProcessAndExtractCommands(BotFd, BotInBuffer);
        } else if (BytesRead < 0) {

            if (errno != EWOULDBLOCK && errno != EAGAIN)
                throw std::runtime_error("recv error.");
        }
    }
}

std::string Trim(const std::string &Str) {
  size_t First = Str.find_first_not_of(" \t\n\r");
  if (std::string::npos == First) {
    return Str;
  }
  size_t Last = Str.find_last_not_of(" \t\n\r");
  return Str.substr(First, (Last - First + 1));
}

bool IsValidNickName(std::string Nickname) {
    if (Nickname.empty()) {
        return false;
    }
    char FirstChar = Nickname[0];
    if (FirstChar == '#' || FirstChar == ':' || std::isdigit(FirstChar))
        return false;
    
    for (size_t I = 0; I < Nickname.size(); I++) {
        char C = Nickname[I];
        if (! (std::isalnum(C) || 
               C == '[' || C == ']' || C == '{' || C == '}' || 
               C == '\\' || C == '|')) 
        {
            return false;
        }
    }
    return true; 
}

bool IsValidPort(const char *PortStr) {
  if (!PortStr || *PortStr == '\0') 
        return false;
  char *EndPtr;
  errno = 0;
  long Port = std::strtol(PortStr, &EndPtr, 10);
  if (*EndPtr != '\0' || errno == ERANGE || Port < 1 || Port > 65535)
        return false;
  return true;
}

bool IsValidPassword(const std::string &Password) {
    if (Password.empty()) {
        std::cerr << "Error: Password cannot be empty." << std::endl;
        return false;
    }
    size_t I = 0;
    for (; I < Password.length(); I++)
    {
      if (!std::isprint(Password[I])) {
        std::cerr << "Error: Password contains unprintable characters." << std::endl;
        return false;
      }
    }
    I = 0;     for (; I < Password.length(); I++)
    {
      if (!std::isspace(Password[I]))
        break;
    }
    if (I == Password.length())
    {
      std::cerr << "Error: Password cannot be empty." << std::endl;
      return false;
    }

    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <nick_name> <server_ip> <port> <password>" << std::endl;
        return 1;
    }
    
    std::string NickName = Trim(argv[1]);
    if (!IsValidNickName(NickName)) {
        std::cerr << "Error: Invalid Nickname format." << std::endl;
        return 1;
    }
    
    std::string ServerIp = Trim(argv[2]);
    if (ServerIp.empty()) {
        std::cerr << "Error: Server IP cannot be empty." << std::endl;
        return 1;
    }
    
    std::string PortStr = Trim(argv[3]);
    if (!IsValidPort(PortStr.c_str())) {
        std::cerr << "Error: Invalid port. Must be 1-65535." << std::endl;
        return 1;
    }
    
    int Port = std::atoi(PortStr.c_str()); 
    std::string Password = Trim(argv[4]);
    
    if (!IsValidPassword(Password)) {
        return 1;
    }
    
    int BotFd = -1;
    try {
        BotFd = SetupConnection(ServerIp, Port, Password, NickName);
        StartBotLoop(BotFd);

    } catch (const std::exception &e) {
        std::cerr << "Bot Fatal Error: " << e.what() << std::endl;
        if (BotFd > 0)
            close(BotFd);
        return 1;
    }
    return 0;
}
