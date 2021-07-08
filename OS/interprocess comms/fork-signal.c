#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

void handle(int arg) 
{
	printf("oh hi mark %d\n", arg);
    exit(EXIT_SUCCESS);
}

int main(void)
{
    struct
    {
        unsigned int count:2;
    } counter;
    char indicators[] = {'-', '\\', '|', '/'};

    pid_t pid = fork();

    if (!pid)
    {
        signal(SIGINT, handle);
        while(1)
        {
            printf("\rIn the child, waiting: %c.", indicators[counter.count++]);
            fflush(stdout);
            sleep(1);
        }
    }
    else
    {
        printf("In the parent, going to sleep\n");
        //while(1);
        sleep(5);
        printf("Sending a signal.\n");
        kill(pid, SIGINT);
    }
	return EXIT_SUCCESS;
}

