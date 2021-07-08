CC = clang
CFLAGS = -Wall -Wpedantic -Wextra -Werror -g -lm
LDFLAGS = -lpthread

all: sample-threads sample-processes

sample-threads: sample.c
	$(CC) $(CFLAGS) $(LDFLAGS) sample.c -o sample-threads -DUSE_THREADS

sample-processes: sample.c
	$(CC) $(CFLAGS) $(LDFLAGS) sample.c -o sample-processes -DUSE_PROCS
  
clean:
	rm -f sample-threads sample-processes 

