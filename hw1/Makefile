# Define (atleast) the following targets: all, clean
# all must build the executable file named mapreduce.
# clean must remove all object files and executable files created.
# Look at the introduction doc for information on how make a Makefile.
# This make file must build according to the structure described in the
# introduction doc, if it doesn't you will get a ZERO!
CFLAGS = -Wall -Werror
INCLUDE = ./include/
MAKEDIR = mkdir -p
HEADERS = include/const.h include/map_reduce.h

all: makedir homework

homework: makeobj
	gcc $(CFLAGS) -I./include -o bin/mapreduce src/main.c 

debug:
	gcc -g -I./include $(CFLAGS) -o bin/mapreduce src/main.c

makedir:
	$(MAKEDIR) bin build

makeobj:
	gcc $(CFLAGS) -I./include -c -o build/main.o src/main.c 
	gcc $(CFLAGS) -I./include -c -o build/map_reduce.o src/map_reduce.c

main.o: src/main.c $(HEADERS)
	gcc $(CFLAGS) -I./include -c -o build/main.o src/main.c

map_reduce.o: src/map_reduce.c
	gcc $(CFLAGS) -I./include -c -o build/map_reduce.o src/map_reduce.c
	
clean:
	rm -rf bin build