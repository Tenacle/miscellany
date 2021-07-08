
#include <stdio.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

void signalHandler(int sig)
{
    //fprintf(stdout, "signal handler received: %d \n", sig);
    fprintf(stdout, "signal handler received: %d (%d)\n", sig, SIGUSR1);
    switch(sig)
    {
        case SIGUSR1:
            fprintf(stdout, "got SIGUSR1\n");
            break;
        case SIGUSR2:
            fprintf(stdout, "got SIGUSR2\n");
            break;
    }
}


int main(int argc, char **argv)
{
    int fd[2];

    if(argc < 2)
    {
        fprintf(stderr, "Need an argument");
        exit(EXIT_FAILURE);
    }

    if(pipe(fd) == -1)
    {
        perror("pipe");
    }

    fprintf(stdout, "Running arg(%d):\n", argc-1);
    for(int i=1; i < argc; i++)
    {
        printf("%s, ", argv[i]);
    }
    printf("\n");


    int pid = fork();
    int WRITE_END = 1;
    int READ_END = 0;
    char buffer[1];

//    int status;

    
    if(pid == 0)
    {
        // signal 
//        printf("child here, sending signals to %d\n", parentpid);
//        kill(parentpid, SIGUSR2);
//        //kill(parentpid, SIGUSR2);

        // pipes - we want to write output to parent
        while((dup2(fd[WRITE_END], STDOUT_FILENO) == -1) &&
                (errno == EINTR)) {}
        close(fd[WRITE_END]);
        close(fd[READ_END]);
        execvp(argv[1], argv+1);
        perror("execp");
        exit(1);


//        close(fd[READ_END]);        // close read end
        //execvp("ls\0", "~\0");
        //char *str = "hello worlds 123456789";
//        close(fd[WRITE_END]);
    }
    else
    {
        // signal
//        printf("parent here here (%d), waiting for signals\n", parentpid);
//        signal(SIGUSR1, signalHandler);
//        signal(SIGUSR2, signalHandler);
//        while(NULL);

        // pipes
//        waitpid(pid, &status, 0);   // maybe this will be signals?
//        close(fd[WRITE_END]);
//        int c2 = dup2(fd[READ_END], STDIN_FILENO);
        //printf("parent dup2 read: %d\n", c2);
        close(fd[WRITE_END]);
        while(1)
        {
            ssize_t count = read(fd[READ_END], buffer, sizeof(buffer));
            if(count == -1) {
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
                break;
            }
            else
            {
                //fprintf(stdout, "count:%zd\n", count);
                //printHandler(buffer, count);
                fprintf(stdout, "%s\n", buffer);
            }
        }
        close(fd[READ_END]);
        exit(EXIT_SUCCESS);
        wait(NULL);
    }
}

