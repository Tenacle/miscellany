/*
  Student Name: Tristan
  Student ID: 7818872
  Section: A01
--------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

// easy differentiate write and read pipes
const int WRITE_END = 1;
const int READ_END = 0;
// states that change depending on the signals.
typedef enum {CHILD, PARENT, HAPPY, SAD} status;
status state = PARENT;                     // defaults to cat-to-dog 


/* signal handlers*/
void signalHandler(int signum)
{
    switch(signum)
    {
        // ignore SIGINT
        case SIGUSR1:
            write(STDOUT_FILENO, "SIGUSR1 signal received. printing happy thoughts.\n", 51);
            //printf("SIGUSR1 signal received. printing happy thoughts.\n");
            state = HAPPY;
            break;
        case SIGUSR2:
            write(STDOUT_FILENO, "SIGUSR2 signal received. printing sad thoughts.\n", 41);
            //printf("SIGUSR2 signal received. printing sad thoughts.\n");
            state = SAD;
            break;
        case SIGALRM:
            write(STDOUT_FILENO, "SIGALRM signal received. doing a switch\n", 50);
            //printf("SIGALRM signal received. Disabling SIGINT and SIGTSTP\n");
            if(state == PARENT)
            {
                state = CHILD;
            }
            else
            {
                state = PARENT;
            }
            break;
    }
}


int main()
{
    //pid_t parent_pid = getpid();
    pid_t pid;
    
    // handle signals
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
    signal(SIGALRM, signalHandler);

#ifdef SIGACTION
    // sigaction
    struct signaction sa;
    sa.sa_handler = saHandler;
    memset(&sa, 0, sizeof(sa));
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);
#endif


    // check if fork succeeded
    pid = fork();
    if(pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // inspired by the fork-sign.c from class lecture
    struct
    {
        unsigned int count: 2;
    } counter;
    char indicators[] = {'-', '\\', '|', '/'};

    /*************** child process *****************/
    // - will be executing the process
    if(pid==0)
    {
        // execute command contained in the arguments
        while(1)
        {
            switch(state)
            {
                case HAPPY:
                    printf("happy child..\n");
                    break;
                case SAD:
                    printf("sad child..\n");
                    break;
                case CHILD:
                    printf("\r***Child: I'm the captain now.");
                    printf("\n***Child: sleeping for a bit\n");
                    sleep(5);
                    printf("\n***Child: sending SIGUSR1 to parent twice.\n");
                    kill(pid, SIGUSR1);
                    kill(pid, SIGUSR1);
                    printf("\n***Child: sleeping again\n");
                    sleep(5);
                    printf("\n***Child: sending SIGUSR1 and SIGUSR2 at the same time\n");
                    kill(pid, SIGUSR1);
                    kill(pid, SIGUSR2);
                    printf("\n***Child: sleeping again\n");
                    sleep(5);
                    printf("\n***Child: letting go of control\n");
                    kill(pid, SIGALRM);
                    break;
                case PARENT:
                default:
                    printf("\rChild: %c waiting.. %c", indicators[counter.count], indicators[counter.count++]);
                    break;
            }

            fflush(stdout);
            sleep(1);
        }

    }
    /*************** parent process *****************/
    // - simulating man in the middle attack
    else
    {
//#ifdef DEBUG
        printf("parent pid: %d\n", getpid());
//#endif
        // start parsing the chars from the pipe
        while(1)
        {
            switch(state)
            {
                case PARENT:
                    printf("\n***Parent: sleeping for a bit\n");
                    sleep(5);
                    printf("\n***Parent: sending SIGUSR1 to child.\n");
                    kill(pid, SIGUSR1);
                    kill(pid, SIGUSR1);
                    printf("\n***Parent: sleeping again\n");
                    sleep(5);
                    printf("\n***Parent: sending SIGUSR2 to child.\n");
                    kill(pid, SIGUSR2);
                    printf("\n***Parent: sleeping again\n");
                    sleep(5);
                    printf("\n***Parent: sending SIGUSR1 and SIGUSR2 at the same time\n");
                    kill(pid, SIGUSR1);
                    kill(pid, SIGUSR2);
                    printf("\n***Parent: sleeping again\n");
                    sleep(5);
                    printf("\n***Parent: sending SIGUSR1 and SIGALRM at the same time\n");
                    kill(pid, SIGUSR1);
                    kill(pid, SIGALRM);
                    printf("\n***Parent: sleeping again\n");
                    sleep(5);
                    printf("\n***Parent: giving child control\n");
                    kill(pid, SIGALRM);
                    break;
                case CHILD:
                    printf("\nParent: %c waiting.. %c", indicators[counter.count], indicators[counter.count++]);
                    break;
                case HAPPY:
                    printf("Happy parent\n");
                    break;
                case SAD:
                    printf("\nNo. Give me back control");
                    kill(pid, SIGALRM);
                    break;
            }
        }
        sleep(5);
        exit(EXIT_SUCCESS);
    }

}
