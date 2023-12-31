CXX = c++

NAME = webserv

CPPFLAGS = -Wall -Wextra -Werror -std=c++98

SRC =	main.cpp				\
		src/Location.cpp		\
		src/Server.cpp			\
		src/parser/Lexer.cpp	\

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CPPFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re