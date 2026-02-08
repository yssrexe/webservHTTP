CPP = c++
FLAGS = -Wall -Wextra -Werror -std=c++98
SOURCE =  	src/ConfigParser/Config.cpp src/ConfigParser/Route.cpp\
			main.cpp  \
			src/clsServer.cpp \
			src/clsRequest.cpp \
			src/clsResponse.cpp \
			src/clsRounting.cpp \
			src/clsPostBodyFileHandler.cpp \
			src/MyLabrary.cpp \
			src/cgi/handleCGI.cpp \
			src/cgi/ParserCgi/RequestCgi.cpp 

OBJ = $(SOURCE:.cpp=.o)
NAME = webserv

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(FLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CPP) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

# REDIRECT=301-/www.youtube.com

