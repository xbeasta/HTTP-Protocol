FLAG=T910=0

OPTS=-fsanitize=address -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Werror -std=c17 -Wpedantic -O0 -g -pthread

all: hw5

hw5: hw5.o main.o
	gcc -D${FLAG} ${OPTS} main.o hw5.o -o hw5

main.o: main.c hw5.o
	gcc -D${FLAG} ${OPTS} -c main.c -o main.o

hw5.o: hw5.c
	gcc -D${FLAG} ${OPTS} -c hw5.c -o hw5.o

clean:
	rm -rf *.o hw5
