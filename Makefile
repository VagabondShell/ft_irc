NAME      = ircserv
BONUS_NAME  = bot
CC        = c++
CFLAGS    =  -Wall -Wextra -Werror -std=c++98 -fsanitize=address
RM        = rm -rf

SRCS_FILES      = main.cpp Server.cpp Client.cpp AuthCommands.cpp Channel.cpp commands/mode.cpp \
				  commands/privmsg.cpp commands/join.cpp commands/topic.cpp\
				  commands/kick.cpp commands/invite.cpp commands/part.cpp
BONUS_FILE = bonus/bot.cpp 
BONUS_OBJS  = $(BONUS_FILE:.cpp=.o)

SRCS 		= $(addprefix srcs/, $(SRCS_FILES))
OBJS      = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

bonus: $(BONUS_NAME)

$(BONUS_NAME): $(BONUS_OBJS)
	$(CC) $(CFLAGS) $(BONUS_OBJS) -o $(BONUS_NAME)

srcs/%.o: srcs/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -MMD

bonus/%.o: bonus/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(OBJS:.o=.d)
	$(RM) $(BONUS_OBJS) 

fclean: clean
	$(RM) $(NAME)
	$(RM) $(BONUS_NAME)

re: fclean all

-include $(OBJS:.o=.d)

.PHONY: all clean fclean re bonus
