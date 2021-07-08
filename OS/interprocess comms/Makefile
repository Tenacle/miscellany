CC = clang
CFLAGS = -Wall -Wpedantic -Wextra -Werror -g -lm
LDFLAGS = -lpthread

all: a2q1 a2q2

a2q1: a2q1.c
	$(CC) $(CFLAGS) $(LDFLAGS) a2q1.c -o q1

a2q2: a2q2.c
	$(CC) $(CFLAGS) $(LDFLAGS) a2q2.c -o q2
  
clean:
	rm -f q1 q2

