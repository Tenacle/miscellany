CC = clang
#CFLAGS = -Wall -Wpedantic -Wextra -Werror -g
CFLAGS = -Wall -Wpedantic -Wextra -Werror
#LDFLAGS = -lpthread
OFILE = fat32
IFILE = fat32.c

all: file

file: $(IFILE)
#	$(CC) $(CFLAGS) $(LDFLAGS) $(IFILE) -o $(OFILE)
	$(CC) $(CFLAGS) $(IFILE) -o $(OFILE)

clean:
	rm -f $(OFILE)
