CC		= g++
CPPVER	= c++20
SRC		= src
FLAGS	= -Wall -Wextra -Werror
OPTI	= -O3
INC		= $(PWD)/include
PROF	= -O0 -pg
DEBUG	= -g

rel: FLAGS += $(OPTI)
rel: order-book

profile: FLAGS += $(PROF)
profile: order-book

debug: FLAGS += $(DEBUG)
debug: order-book

order-book: itch_reader.o order_book.o main.o
	$(CC) -std=$(CPPVER) $(FLAGS) $(SRC)/itch_reader.o $(SRC)/order_book.o $(SRC)/main.o -o order-book -lglog

main.o:	$(SRC)/main.cpp
	$(CC) -std=$(CPPVER) -c -I$(INC) $(FLAGS) $(SRC)/main.cpp -o $(SRC)/main.o

order_book.o:	$(SRC)/order_book.cpp
	$(CC) -std=$(CPPVER) -c -I$(INC) $(FLAGS) $(SRC)/order_book.cpp -o $(SRC)/order_book.o

itch_reader.o:	$(SRC)/itch_reader.cpp
	$(CC) -std=$(CPPVER) -c -I$(INC) $(FLAGS) $(SRC)/itch_reader.cpp -o $(SRC)/itch_reader.o

clean:
	rm --verbose --force $(SRC)/*.o *.out order-book

