all: pdp-11

pdp-11: main.o functions.o memory_manager.o
	gcc -o ./pdp-11 main.o functions.o memory_manager.o

main.o: main.c
	gcc -c main.c

functions.o: functions.c
	gcc -c functions.c

memory_manager.o: memory_manager.c
	gcc -c memory_manager.c

