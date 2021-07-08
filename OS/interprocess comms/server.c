#include <sys/types.h> //mkfifo
#include <sys/stat.h>  //mkfifo
#include <stdio.h>     // printf
#include <fcntl.h>     // open
#include <unistd.h>    // unlink
#include <stdlib.h>    // exit
#include <stdbool.h>   // bool

#define BUFFER_SIZE 2
#define PERMISSIONS 0600

int main(void)
{
	const char SERVER_FIFO[] = "/tmp/COMP3430_pipe";
	char buf[BUFFER_SIZE];
	bool done = false;

	// create the named pipe
 	int result = mkfifo(SERVER_FIFO, PERMISSIONS);

	if (result)
    {
		perror("Unable to create named pipe");
		exit(EXIT_FAILURE);
	}

	// connect to the named pipe
	// see `man 2 open` for access controls
	printf("opening file\n");
	int fd = open(SERVER_FIFO, O_RDONLY);
	if (fd == -1)
    {
		perror("Unable to create named pipe");
		exit(EXIT_FAILURE);
	}
	printf("done opening file\n");

	while(!done)
    {
		// listen, echo to terminal
		result = read(fd, &buf, 1);
		if (result < 0)
        {
			perror("ERROR: Error reading from pipe");
			exit(EXIT_FAILURE);
		}
		else if (result == 0)
        {
			// last client disconnected.
			// end of file
			printf("End of file");
			done = true;
		} 
        else if ('\a' == buf[0]) 
        {
			// bell to quit
			done = true;
		}
		else if ('\0' == buf[0]) 
        {
			printf("\n");
		} 
		else 
        {
			printf("%c", buf[0]);
		}
	}

	unlink(SERVER_FIFO);

    return EXIT_SUCCESS;
}
