CC	= gcc
CFLAGS 	=

all: MergeLogs

MergeLogs: mergeLogs.o
	$(CC) $(LDFLAGS) -o $@ $^

mergeLogs.o: mergeLogs.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean: FRC
	rm -f mergeLogs.o MergeLogs

#this pseudo target works under the assumption there's no file called FRC in the current directory.
FRC: 
