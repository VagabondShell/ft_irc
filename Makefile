NAME      = ircserv
CC        = c++
CFLAGS    =  -Wall -Wextra -Werror -std=c++98 -fsanitize=address
RM        = rm -rf

SRCS_FILES      = main.cpp Server.cpp Client.cpp AuthCommands.cpp  Command.cpp Channel.cpp
SRCS 		= $(addprefix srcs/, $(SRCS_FILES))

OBJS      = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -MMD

clean:
	$(RM) $(OBJS) $(OBJS:.o=.d)

fclean: clean
	$(RM) $(NAME)

re: fclean all

-include $(OBJS:.o=.d)

.PHONY: all clean fclean re
