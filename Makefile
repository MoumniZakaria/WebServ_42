SRC = request/request.cpp \
	  response/response.cpp\
	  request/request_implementation.cpp \
	  request/tools.cpp \
	  c_tools/tools.cpp \
	  client/client.cpp \
	  server/server.cpp \
	  parsing/Confile.cpp\
	  parsing/ServerBlock.cpp\
	  parsing/RouteBlock.cpp\
	  parsing/parsing.cpp\
	  parsing/pars_route.cpp\
	  parsing/check_tools.cpp\
	  main.cpp 


OBJ = $(SRC:.cpp=.o)
NAME = webserv
HEDER = request/request.hpp parsing/Confile.hpp  parsing/ServerBlock.hpp parsing/RouteBlock.hpp
CFLAGS =  -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address

all : $(NAME)

%.o : %.cpp ${HEDER} Makefile
	g++ ${CFLAGS} -c $< -o $@

$(NAME) : $(OBJ) ${HEDER}
	 g++ ${CFLAGS} $(OBJ) -o $@

clean :
	rm -rf $(OBJ)

fclean : clean
	rm -rf $(NAME)

re : fclean all

.PHONY : clean
