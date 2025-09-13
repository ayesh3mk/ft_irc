NAME     := ircserv
CXX      := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -Iinclude

SRC := src/main.cpp src/Server.cpp src/Channel.cpp src/Client.cpp \
     src/INVITE.cpp src/JOIN.cpp src/KICK.cpp src/MODE.cpp \
     src/nick_cmd.cpp src/parser.cpp src/pass_cmd.cpp \
     src/PRIVMSG.cpp src/TOPIC.cpp src/user_cmd.cpp

OBJ := $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re