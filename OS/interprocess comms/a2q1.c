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
typedef enum {DOGTOCAT, CATTODOG, NORMAL} MITMstate;
MITMstate state = CATTODOG;                     // defaults to cat-to-dog 


/* signal handlers*/
void signalHandler(int signum)
{
    switch(signum)
    {
        // replace any instance of 'dog' with 'cat'
        case SIGUSR1:
#ifdef DEBUG
            printf("SIGUSR1 signal received. dog -> cat\n");
#endif
            state = DOGTOCAT;
            break;
        // replace any instance of 'cat' with 'dog'
        case SIGUSR2:
#ifdef DEBUG
            printf("SIGUSR2 signal received. cat -> dog\n");
#endif
            state = CATTODOG;
            break;
        // nothing is replace
        case SIGALRM:
#ifdef DEBUG
            printf("SIGALRM signal received. reverting to normal mode\n");
#endif
            state = NORMAL;
            break;
    }
}

int main(int argc, char **argv)
{
    pid_t pid;
    int fd[2];
    char buffer[1];                             
    // handle signals
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
    signal(SIGALRM, signalHandler);

    

    /* check error */
    // check if an argument was passed
    if(argc < 2)
    {
        fprintf(stderr, "Need at least one more arg");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    printf("Running arg(%d): ", argc-1);
    for(int i=1; i < argc-1; i++)
    {
        printf("%s, ", argv[i]);
    }
    printf("%s\n", argv[argc-1]);
#endif

    // check if pipe succeeded
    if(pipe(fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    // check if fork succeeded
    pid = fork();
    if(pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    /*************** child process *****************/
    // - will be executing the process
    if(pid==0)
    {
        close(fd[READ_END]);                    // close pipe read. not needed.
        // Handle interrupts by ignoring them. keep doing till dup2 is successfull
        while((dup2(fd[WRITE_END], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
        // we've connected the pipe so we can close the file now.
        close(fd[WRITE_END]);                   // close pipe write
        execvp(argv[1], argv+1);
        perror("execp");
        exit(1);
    }
    /*************** parent process *****************/
    // - simulating man in the middle attack
    else
    {
        close(fd[WRITE_END]);                   // close pipe write (don't need)
#ifdef DEBUG
        printf("parent pid: %d\n", getpid());
#endif
        // start parsing the chars from the pipe
        while(1)
        {
            ssize_t count = read(fd[READ_END], buffer, sizeof(buffer));
            if(count == -1) {
                // Interrupt handling. We chose to ignore the interrupt and redo operation.
                if(errno == EINTR)
                {
                    continue;
                }
                else
                {
                    perror("read");
                    exit(1);
                }
            }
            else if(count == 0)
            {
                break;          // exhausted the pipe. We are done.
            }
            else
            {
                /* start of MITM attack. Defaults to CATTODOG */
                // (default) cat-to-dog state. Replace 'cat' words to 'dog'.
                if(state == CATTODOG && buffer[0] == 'c')
                {
                    char cat[3];
                    read(fd[READ_END], cat, 2);             // only need the last two chars of "cat" since c was found
                    cat[2] = '\0';
                    if((strcmp(cat, "at") == 0))
                    {
                        printf("dog");
                    }
                    else
                    {
                        printf("%c%s", buffer[0], cat);     // print what we read without change. not what we're looking for.
                    }
                    continue;
                }
                // dog-to-cat state. Replace 'dog' words to 'cat'.
                else if (state == DOGTOCAT && buffer[0] == 'd')
                {
                    char dog[3];
                    read(fd[READ_END], dog, 2);             // same as with "cat"
                    dog[2] = '\0';
                    if(strcmp(dog, "og") == 0)
                    {
                        printf("cat");
                    }
                    else
                    {
                        printf("%c%s", buffer[0], dog);
                    }
                    continue;
                }
                // normal state. Will not replace  anything.
                else
                {
                printf("%s", buffer);
                }
            }
        }
        close(fd[READ_END]);            // we're done reading the pipe now
    }

}
