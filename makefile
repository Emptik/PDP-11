all: pdp-11

pdp-11: main.o functions.o memory_manager.o
	gcc -Wall -o ./pdp-11 main.o functions.o memory_manager.o

main.o: main.c
	gcc -Wall -c main.c

functions.o: functions.c
	gcc -Wall -c functions.c

memory_manager.o: memory_manager.c
	gcc -Wall -c memory_manager.c

