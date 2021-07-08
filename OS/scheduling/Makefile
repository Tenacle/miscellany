CC = clang
CFLAGS = -Wall -Wpedantic -Wextra -Werror -g -lm
LDFLAGS = -lpthread
#FLAGS = -DDEBUG

#all: a3q1 a3q2
all: a3q1

a3q1: a3q1.c
	$(CC) $(CFLAGS) $(LDFLAGS) a3q1.c -o q1 $(FLAGS)

#a3q2: a3q2.c
#	$(CC) $(CFLAGS) $(LDFLAGS) a3q2.c -o q2
  
clean:
	rm -f q1 q2

