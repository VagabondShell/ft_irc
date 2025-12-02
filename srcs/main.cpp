#include "../includes/Server.hpp"
bool isValidPort(const char *port_str) {
  if (!port_str || *port_str == '\0') 
        return false;
  char *endptr;
  errno = 0;
  long port = std::strtol(port_str, &endptr, 10);
  if (*endptr != '\0' || errno == ERANGE || port < 1 || port > 65535)
        return false;
  return true;
}
int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Error: Usage: " << argv[0]
              << " \"./ircserv <port> <password>\"" << std::endl;
    return 1;
  }
  int port;
  try {
    if (isValidPort(argv[1]))
      port = atoi(argv[1]);
    else{
      std::cerr << "Error: Invalid port. Range is 1-65535." << std::endl;
      return 1;
    }
    const std::string password = argv[2];
    Server irc_server(port, password);
    irc_server.run();

  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Error: An unknown exception occurred during RPN processing."
              << std::endl;
    return 1;
  }
  return 0;
}
