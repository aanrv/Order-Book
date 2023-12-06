CC		= g++
CPPVER	= c++14
SRC		= src
FLAGS	= -Wall -Wextra -Werror
INC		= /home/varan/Documents/Order-Book

order-book: itch_parser.o main.o
	$(CC) -std=$(CPPVER) $(FLAGS) $(SRC)/itch_parser.o $(SRC)/main.o -o order-book

main.o:	$(SRC)/main.cpp
	$(CC) -std=$(CPPVER) -c -I$(INC) $(FLAGS) $(SRC)/main.cpp -o $(SRC)/main.o

itch_parser.o:	$(SRC)/itch_parser.cpp
	$(CC) -std=$(CPPVER) -c -I$(INC) $(FLAGS) $(SRC)/itch_parser.cpp -o $(SRC)/itch_parser.o

clean:
	rm --verbose --force $(SRC)/*.o order-book

