CC		= g++
CPPVER	= c++20
SRC		= src
FLAGS	= -Wall -Wextra -Werror
INC		= $(PWD)
PROF	= -O0 -pg

all: order-book

profile: FLAGS += $(PROF)
profile: all

order-book: order_book.o main.o itch_reader.o
	$(CC) -std=$(CPPVER) $(FLAGS) $(SRC)/itch_reader.o $(SRC)/main.o -o order-book

main.o:	$(SRC)/main.cpp
	$(CC) -std=$(CPPVER) -c -I$(INC) $(FLAGS) $(SRC)/main.cpp -o $(SRC)/main.o

order_book.o:	$(SRC)/order_book.cpp
	$(CC) -std=$(CPPVER) -c -I$(INC) $(FLAGS) $(SRC)/order_book.cpp -o $(SRC)/order_book.o

itch_reader.o:	$(SRC)/itch_reader.cpp
	$(CC) -std=$(CPPVER) -c -I$(INC) $(FLAGS) $(SRC)/itch_reader.cpp -o $(SRC)/itch_reader.o

clean:
	rm --verbose --force $(SRC)/*.o *.out order-book

