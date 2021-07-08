/*--------------------------------------------------------------------
  Student Name: Tristan Montilla
  Student ID: 7818872
  Section: A01
--------------------------------------------------------------------*/
#include <stdio.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#define WORKERS 32
#define MAX_WORKERS 2

// where I'll be doing the work I guess
static void *hard_work(void *work)
{
    //printf("&work @ hard_work(): %p\n", work);
    int *amount = (int*)malloc(sizeof(int));                                // store the sum for this job's computation
    int k = *(int *) work;
    *((int *)work) += 1;                                                     // claim the iteration
    struct timespec t = { .tv_sec = 0, .tv_nsec = *amount * 100000 };
    nanosleep( &t, NULL ); // **Really** hard work.
    

    /* MY CODE */
    long double K = 6;
    long double M = 1;
    long double L = 13591409;
    long double X = 1;
    long double S = 13591409;

    // computation
    M = (pow(K, 3) - 16*K) * M;
    M = floor(M / pow(k, 3));
    L += 545140134 * k;
    X *= -262537412640768000 * k;
    S += M*L/X * k;
    K += 12 * k;
    
    return amount;
}

struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;

	if ( (end.tv_nsec-start.tv_nsec) < 0)
	{
		temp.tv_sec = end.tv_sec-start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	}
	else
	{
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}

	return temp;
}


#ifdef USE_PROCS
#define worker_t pid_t

#define create_worker(work) make_fork(work);
#define stop_worker(worker, sum) kill_fork(worker, sum);

static pid_t make_fork(int *work)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Failed to fork new process.");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)                                                              // child process
    {
        //long double *amount = hard_work(work);
        hard_work(work);
        exit(EXIT_SUCCESS);
    }
    return pid;
}
static long double *kill_fork(pid_t pid, void *sum)
{
    waitpid(pid, NULL, 0);
    return sum;
}
#endif

//*** Threads **********************************************

#ifdef USE_THREADS
#define worker_t pthread_t *

#define create_worker(work) make_thread(work);
#define stop_worker(worker, sum) kill_thread(worker, sum);

static pthread_t *make_thread(int *work)                                            // creates thread
{
    //printf("&work @ make_thread(): %p\n", work);
    pthread_t *thread = malloc(sizeof(pthread_t));
    pthread_create(thread, NULL, hard_work, work);
    return thread;
}

static long double *kill_thread(pthread_t *thread, void *sum)
{
    //printf("&sum @ kill_thread(): %p\n", sum);
    //printf("*sum value b4 join: %d\n", *(int *) sum);
    pthread_join(*thread, &sum);
    //printf("*sum value @ join: %d\n", *(int *) sum);
    free(thread);
    return sum;
}
#endif

int main(void)
{
    //int *work = malloc(sizeof(int));
    int work = 1;
    unsigned int current_workers = 0;
    worker_t workers[MAX_WORKERS];
	struct timespec start, end, total;

    long double sum = 0;

	clock_gettime(CLOCK_REALTIME, &start);
    for (int i = 0; i < WORKERS; i++)
    {
        workers[current_workers] = create_worker(&work);                
        //printf("&work @ main(): %p\n", &work);
        //printf("work: %d\n", work);
        current_workers++;

        if (current_workers == MAX_WORKERS)
        {
            for (int j = 0; j < MAX_WORKERS; j++)
            {
                sum += *stop_worker(workers[j], &sum); 
                //printf("&sum @ main (): %p\n", &sum);
                //printf("*sum value after join: %d\n", sum);
            }
            current_workers = 0;
        }
    }
    sum = 426880 * sqrt(10005) / sum;
    printf("\nFINISHED. Amount is: %.1000LF\n", sum);

	clock_gettime(CLOCK_REALTIME, &end);
    total = diff(start, end);

    printf("Finished experiment with timing %lds, %ldns\n", total.tv_sec, total.tv_nsec);
    printf("Created %d total workers in that time.\n", WORKERS / MAX_WORKERS);

    return EXIT_SUCCESS;
}
