CC		= g++
CPPVER	= c++14
SRC		= src
FLAGS	= -Wall -Wextra -Werror
INC		= /home/varan/Documents/Order-Book

order-book: itch_reader.o main.o
	$(CC) -std=$(CPPVER) $(FLAGS) $(SRC)/itch_reader.o $(SRC)/main.o -o order-book

main.o:	$(SRC)/main.cpp
	$(CC) -std=$(CPPVER) -c -I$(INC) $(FLAGS) $(SRC)/main.cpp -o $(SRC)/main.o

itch_reader.o:	$(SRC)/itch_reader.cpp
	$(CC) -std=$(CPPVER) -c -I$(INC) $(FLAGS) $(SRC)/itch_reader.cpp -o $(SRC)/itch_reader.o

clean:
	rm --verbose --force $(SRC)/*.o order-book

