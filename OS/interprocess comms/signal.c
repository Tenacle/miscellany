#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

void handle(int arg) 
{
	printf("oh hi mark\n");
}
int main(void) 
{
    struct
    {
        unsigned int count:2;
    } counter;
    char indicators[] = {'-', '\\', '|', '/'};

	signal(SIGINT, handle);
    signal(SIGKILL, handle);
	while (1)
    {
        printf("\r%c", indicators[counter.count++]);
        fflush(stdout);
        sleep(1);
    }
	return EXIT_SUCCESS;
}

