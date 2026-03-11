CC=gcc
CFLAGS=-Wall -Wextra

p6: p6.c
	$(CC) $(CFLAGS) -o p6 p6.c

clean:
	rm -f p6