NAME = ircserv

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -g -std=c++98

SRC = $(addprefix ./src/, $(SOURCES))

SOURCES = main.cpp Server.cpp Client.cpp Commands.cpp Utils.cpp Channel.cpp SignalHandler.cpp

DEBUGFLAGS = -g3

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

debug: CXXFLAGS += $(DEBUGFLAGS) -Wall -Werror -Wextra
debug: $(NAME)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $@
	@echo "ircserv built"

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all debug clean fclean re