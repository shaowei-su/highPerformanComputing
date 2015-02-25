all: gauss

CC=gcc
CCFLAGS = -O3
LDFLAGS =

gauss: gauss.o
	$(CC) $(LDFLAGS) -o gauss gauss.o

gauss.o: gauss.c
	$(CC) $(CCFLAGS) -c gauss.c -o gauss.o

clean:
	-rm *.o gauss
