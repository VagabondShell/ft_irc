#ifndef COMMAND_HPP
#define COMMAND_HPP

enum e_cmd_type {
  CMD_UNKNOWN = 0,
  CMD_PASS    = 1,
  CMD_NICK    = 2,
  CMD_USER    = 3,
  CMD_PONG    = 4,
  CMD_PRIVMSG  = 5,
  CMD_MODE    = 6,
  CMD_JOIN    = 7,
  CMD_INVITE  = 8,
  CMD_KICK    = 9,
  CMD_TOPIC   = 10,
  CMD_PART    = 11,
};

//bot command
enum e_cmd_bot_type {
    BOT_CMD_HELP    = 0, // Displays available commands
    BOT_CMD_TIME    = 1, // Displays the server time
    BOT_CMD_JOKE  = 2, 
    BOT_CMD_UNKNOWN = 4, // Mandatory default for unrecognized commands
};


#endif // !COMMAND_HPP
