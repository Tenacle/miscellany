#include <sys/types.h> //mkfifo
#include <sys/stat.h>  //mkfifo
#include <stdio.h>     // printf
#include <fcntl.h>     // open
#include <unistd.h>    // unlink
#include <stdlib.h>    // exit
#include <stdbool.h>   // bool
#include <string.h>    // strlen

#define BUFFER_SIZE 64

int main(void)
{

	const char SERVER_FIFO[] = "/tmp/COMP3430_pipe";

    bool done = false;
    bool killserver = false;
    char* result;
    int errno;
    char buf[BUFFER_SIZE];

 	int fifo = open(SERVER_FIFO, O_WRONLY);
	if (fifo < 0)
    {
		perror("Unable to open named pipe");
		exit(EXIT_FAILURE);
	}

    // listen for input
    while (!done)
    {
        result = fgets(buf, BUFFER_SIZE, stdin);

        if (result != NULL) {
            // send this string to the FIFO pipe
            // we want NUL-terminated strings so hack on a \0
            buf[strlen(buf)-1] = '\0';

            if (strcmp(buf, "exit") == 0)
            {
                done = true;
                killserver = true;
            }
            else
            {
                printf("sending %s\n", buf);
                errno = write(fifo, &buf, strlen(buf)+1);
                
                if (errno < 0)
                {
                    perror("ERROR: Error writing to pipe");
                }
            }
        }
        else
            done = true;
    }

    if (killserver)
        write(fifo, "\a", 1);

    // though it's tempting, don't close the fifo
    // leave it open for others to use!
    // Try uncommenting to see what happens.
    //close(fifo);
    return EXIT_SUCCESS;
}
