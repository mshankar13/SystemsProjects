CC = gcc
BIN = utfconverter
CFLAGS = -Wall -Werror -pedantic -Wextra 
DEBUG = -g
SRC = ./src/utfconverter.c
INCLUDE = ./include/
MAKEDIR = mkdir -p
HEADERS = include/utfconverter.h

all: makedir $(BIN)

debug: 
	$(CC) $(DEBUG) $(CFLAGS) -I $(INCLUDE) -o bin/utf $(SRC)

$(BIN): makeobj
	$(CC) $(CFLAGS) -I $(INCLUDE) -o bin/utf $(SRC)

makedir:
	$(MAKEDIR) bin build

makeobj:
	gcc $(CFLAGS) -I./include -c -o build/utf.o $(SRC)

utf.o: $(SRC) $(HEADERS)
	gcc $(CFLAGS) -I./include -c -o build/utf.o $(SRC)

.PHONY: clean

clean:
	rm -rf bin build

