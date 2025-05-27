CC = gcc
CFLAGS = -Wall -g -pthread
LDFLAGS = -pthread
OBJ = main_rr.o schedulers_rr.o task.o list.o timer.o

all: rr_program

rr_program: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o rr_program $(LDFLAGS)

main_rr.o: main_rr.c schedulers_rr.h task.h list.h timer.h
	$(CC) $(CFLAGS) -c main_rr.c

schedulers_rr.o: schedulers_rr.c schedulers_rr.h task.h list.h timer.h
	$(CC) $(CFLAGS) -c schedulers_rr.c

task.o: task.c task.h
	$(CC) $(CFLAGS) -c task.c

list.o: list.c list.h task.h
	$(CC) $(CFLAGS) -c list.c

timer.o: timer.c timer.h
	$(CC) $(CFLAGS) -c timer.c

clean:
	rm -f *.o rr_program
