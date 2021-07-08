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
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>

#ifdef DEBUG
# define DEBUG_TASK true
#else
# define DEBUG_TASK false
#endif
#define DEBUG_PRINT(fmt, ...) \
    do { if (DEBUG_TASK) fprintf(stderr, fmt, __VA_ARGS__); } while (0)
#define BUFFER_SIZE 256
#define NUM_PROCESS_TYPE 4
#define NUM_PROCESS_PRIORITY 3
#define TIMESLICE 5
#define PROCESS_FIELDS 5

typedef enum {RR=1, STCF, PRR, SCHEDULE_TYPE_COUNT} SCHEDULE_TYPE;
typedef enum {SHORT_THREADS, MEDIUM_THREADS, LONG_THREADS, IO_THREADS, THREAD_TYPE_COUNT} THREAD_TYPE;
typedef struct
{
    int process_id;
    char thread_name[BUFFER_SIZE];
    THREAD_TYPE thread_type;    // 0-4
    int priority;               // 0-2; 0 highest, 2 lowest
    int thread_length;          // time left
    int odds_of_IO;             // %
    int start;                  // time when process started
    int finish;                    // time when process ended
    int total;                  // run time (end-start)
} Process;
typedef struct Node{
    Process *data;
    struct Node *next;
} Node;

int pseudo_clock = 0;               // simulating time
SCHEDULE_TYPE sched_type = RR;      // defaults to RR sched
Node *head = NULL;
Node *last = NULL;
int type_counter[NUM_PROCESS_TYPE];
int type_time[NUM_PROCESS_TYPE];
int priority_counter[NUM_PROCESS_PRIORITY];
int priority_time[NUM_PROCESS_PRIORITY];
int process_id = 0;


void print_schedule_type()
{
    switch(sched_type)
    {
        case RR:
            printf("\nUsing pure Round-Robin\n");
            break;
        case STCF:
            printf("\nUsing Shortest Time to Completion First\n");
            break;
        case PRR:
            printf("\nUsing Priority Round-Robin\n");
            break;
        default:
            fprintf(stderr, "\nUnknown scheduling type set. Seting to default (RR) instead.\n");
            sched_type = RR;
            print_schedule_type();
    }
}

void print_process(Process *process)
{
    printf("[%4d, %15s, %d, %d, %4d, %3d%%, %4d, %4d, %4d]\n", process->process_id, process->thread_name, process->thread_type, process->priority, process->thread_length, process->odds_of_IO, process->start, process->finish, process->total);
}

void print_process_queue()
{
    if(head == NULL)
    {
        printf("No processes in queue\n");
        return;
    }
    printf("Printing process queue:\n");
    Node *curr = head;
    print_process(curr->data);
    while(curr->next != NULL)
    {
        curr = curr->next;
        print_process(curr->data);
    }
    printf("Finished printing process queue\n");
}

Node *get_node_at_index(int index)
{
    int counter = 0;
    Node *curr = head;
    while(counter != index)
    {
        if(curr == NULL)
        {
            return NULL;
        }
        curr = curr->next;
        counter++;
    }
    return curr;
}

int remove_node_at_index(int index)
{
//    DEBUG_PRINT("DELETING NODE AT INDEX: %d\n", index);
    int counter = 0;
    Node *curr = head;
    Node *prev = NULL;
    if(curr == NULL)
    {
        return -1;
    }
    if(index == 0)
    {
        head = head->next;
    }
    else
    {
        while((counter != index) && (curr != NULL))
        {
            prev = curr;
            curr = curr->next;
            counter++;
        }
        prev->next = curr->next;
    }
//    if(DEBUG_TASK)
//    {
//        printf("Deleting this process:\n\t");
//        print_process(curr->data);
//        printf("Deleted\n");
//        print_process_queue();
//    }
    free(curr);
    return 0;

}


int getrand(int min, int max)
{
    srand(time(NULL));
    return (rand() % max-min+1) + min;
}

// simulate process running
// processor essentially takes hold of CPU.
// no preemption
int sim_process(Process *process)
{
//    if(DEBUG_TASK)
//    {
//        print_process(process);
//    }
    // check if process is finish time is already declared or if it doesn't need any more processing time.
    if(!(process->finish < 0) || process->thread_length <= 0)
    {
        fprintf(stderr, "Process %15s already completed..\n", process->thread_name);
    }

    bool requires_IO = false;
    int allotted_slice = TIMESLICE;
    if(process->start < 0)
    {
//        if(DEBUG_TASK)
//        {
//            printf("Initiating process:\n\t");
//            print_process(process);
//            printf("\n\tSetting start time @ %d\n", pseudo_clock);
//        }
        process->start = pseudo_clock;
    }

    /* Assumes that the requirement was a typo. Since the name is "odds of IO".
     * - random > than odds_of_IO to perform IO was specified in req which was confusing. (100% odds_of_io means 0% io. which doesn't make sense.)
     * - random < odds_of_io to perform IO is used. (100% odds_of_io means 100% io every slice)
     */

    // begin processing task
    
    while(!requires_IO)
    {
        // IO required
        if(getrand(0, 100) < process->odds_of_IO)                              
        {
            allotted_slice = getrand(0, TIMESLICE);
            requires_IO = true;
//            DEBUG_PRINT("Process %15s requires IO. Running %d instead\n", process->thread_name, allotted_slice);
        }
        
        //TODO: See if we need to halt a process when IO is required even tho allotted time completes the whole thing.
        // if alloted slice covers the whole remaining thread_length, then we will declare it as completed (even if it is performing IO)
        // process completed
        if(process->thread_length <= allotted_slice)
        {
            pseudo_clock += process->thread_length;
            process->finish = pseudo_clock;
            process->total = process->finish - process->start;
            process->thread_length = 0;
            if(DEBUG_TASK)
            {
                printf("Finishing process:\n\t");
                print_process(process);
                printf("\tSetting end time @ %d with total: %d from start %d\n\n", pseudo_clock, process->total, process->start);
            }
            return 0;
        }
        // process has done work but is not yet complete
        else
        {
            pseudo_clock += allotted_slice;
            process->thread_length -=  allotted_slice;
        }
    }
    return -1;
}

// RR simulation
void sim_process_queue_handler_RR()
{
    if(head == NULL)
    {
        fprintf(stderr, "No processes to run.\n");
        return;
    }
    Node *curr = head;
    int index = 0;              // will be minding our index in the process

    while(head != NULL)
    {
//        DEBUG_PRINT("(%4d) Handling process #%d\n", pseudo_clock, index);
        // handle process
        int state = sim_process(curr->data);
        
//        DEBUG_PRINT("State after process #%d: %d\n", index, state);
        // process completed successfully. Get stats and remove from queue
        if(state == 0)
        {
//            DEBUG_PRINT("Process #%d type: %d, priority: %d\n", index, curr->data->thread_type, curr->data->priority);
            type_time[curr->data->thread_type] += curr->data->total;
            type_counter[curr->data->thread_type]++;
            priority_time[curr->data->priority] += curr->data->total;
            priority_counter[curr->data->priority]++;
//            DEBUG_PRINT("Stat: type counter: %d, priority counter: %d\n", type_counter[curr->data->thread_type], priority_counter[curr->data->priority]);

            if(DEBUG_TASK)
            {
                print_process_queue();
            }
            // delete process
            curr = curr->next;                  // move to the next process
            remove_node_at_index(index);        // index will be decremented so that we can use the same index (since -1)
        }
        else
        {
            curr = curr->next;
            index++;
        }

        if(curr == NULL)
        {
            curr = head;
            index = 0;
        }
    }
    DEBUG_PRINT("%s", "All process handled successfully\n");
}

void refresh_STCF_queue()
{
    Node *new_head = NULL;
    Node *new_last = NULL;
    Node *tmp = NULL;
    Node *old = head;
    Node *curr = NULL;
    Node *prev = NULL;

    if(old != NULL)
    {
        tmp = malloc(sizeof(Node));
        tmp->data = old->data;
        tmp->next = NULL;
        new_head = tmp;
        new_last = tmp;
        curr = tmp;
        old = old->next;
    }
    while(old != NULL)
    {
        tmp = malloc(sizeof(Node));
        tmp->data = old->data;
        tmp->next = NULL;
        // check if it's the lowest 
        if(tmp->data->thread_length > new_last->data->thread_length)
        {
            new_last->next = tmp;
            new_last = tmp;
        }
        // check if it's the highest
        else if(tmp->data->thread_length < new_head->data->thread_length)
        {
            tmp->next = new_head;
            new_head = tmp;
        }
        // check if it's in the middle
        else
        {
            prev = new_head;
            curr = new_head->next;
            while(curr != NULL)
            {
                if(tmp->data->thread_length < curr->data->thread_length)
                {
                    prev->next = tmp;
                    tmp->next = curr;
                    break;
                }
                prev = curr;
                curr = curr->next;
            }
        }
        old = old->next;
    }

    while(head != NULL)
    {
        Node *remove = head;
        head = head->next;
        free(remove);
    }
    head = new_head;
    last = new_last;
}

void sim_process_queue_handler_STCP()
{
    if(head == NULL)
    {
        fprintf(stderr, "No processes to run.\n");
        return;
    }
    Node *curr = head;
    int index = 0;              // will be minding our index in the process
    int state;
    int id;

    while(head != NULL)
    {
//        DEBUG_PRINT("(%4d) Handling process #%d\n", pseudo_clock, index);
        // handle process
        state = sim_process(curr->data);
        id = curr->data->process_id;
        
//        DEBUG_PRINT("State after process #%d: %d\n", index, state);
        // process completed successfully. Get stats and remove from queue
        if(state == 0)
        {
//            DEBUG_PRINT("Process #%d type: %d, priority: %d\n", index, curr->data->thread_type, curr->data->priority);
            type_time[curr->data->thread_type] += curr->data->total;
            type_counter[curr->data->thread_type]++;
            priority_time[curr->data->priority] += curr->data->total;
            priority_counter[curr->data->priority]++;
//            DEBUG_PRINT("Stat: type counter: %d, priority counter: %d\n", type_counter[curr->data->thread_type], priority_counter[curr->data->priority]);

            if(DEBUG_TASK)
            {
                print_process_queue();
            }
            // delete process
            curr = curr->next;                  // move to the next process
            remove_node_at_index(index);        // index will be decremented so that we can use the same index (since -1)
        }

        // cycle on to the next shortest time to completion process
        refresh_STCF_queue();
        if(head != NULL)
        {
            if((id == head->data->process_id) && (head->next != NULL))
            {
                curr = head->next;
                index = 1;
                continue;
            }
            curr = head;
            index = 0;
        }
    }
    DEBUG_PRINT("%s", "All process handled successfully\n");
}

void sim_process_queue_handler_PRR()
{
    if(head == NULL)
    {
        fprintf(stderr, "No processes to run.\n");
        return;
    }
    Node *curr = head;
    int index = 0;              // will be minding our index in the process
    int state;
    int id;

    while(head != NULL)
    {
//        DEBUG_PRINT("(%4d) Handling process #%d\n", pseudo_clock, index);
        // handle process
        state = sim_process(curr->data);
        id = curr->data->process_id;
        
//        DEBUG_PRINT("State after process #%d: %d\n", index, state);
        // process completed successfully. Get stats and remove from queue
        if(state == 0)
        {
//            DEBUG_PRINT("Process #%d type: %d, priority: %d\n", index, curr->data->thread_type, curr->data->priority);
            type_time[curr->data->thread_type] += curr->data->total;
            type_counter[curr->data->thread_type]++;
            priority_time[curr->data->priority] += curr->data->total;
            priority_counter[curr->data->priority]++;
//            DEBUG_PRINT("Stat: type counter: %d, priority counter: %d\n", type_counter[curr->data->thread_type], priority_counter[curr->data->priority]);

            if(DEBUG_TASK)
            {
                print_process_queue();
            }
            // delete process
            curr = curr->next;                  // move to the next process
            remove_node_at_index(index);        // index will be decremented so that we can use the same index (since -1)
        }
        else
        {
            curr = curr->next;
            index++;
        }

        if((head != NULL))
        {
            if((curr != NULL) && (curr->data->priority > head->data->priority))
            {
                if((id == head->data->process_id) && (head->next != NULL))
                {
                    curr = head->next;
                    index = 1;
                    continue;
                }
            }
            curr = head;
            index = 0;
        }
    }
    DEBUG_PRINT("%s", "All process handled successfully\n");
}

void sim_process_queue()
{
    switch(sched_type)
    {
        case RR:
            sim_process_queue_handler_RR();
            break;
        case STCF:
            sim_process_queue_handler_STCP();
            break;
        case PRR:
            sim_process_queue_handler_PRR();
            break;
        default:
            fprintf(stderr, "\nUnknown scheduling type set. Seting to default (RR) instead.\n");
            sched_type = RR;
            sim_process_queue();
    }
}

void queue_process(Process *process)
{
    Node *this_process = malloc(sizeof(Node));
//    Node *this_process = NULL;
    this_process->data = process;
    this_process->next = NULL;
    if(head == NULL)
    {
        head = this_process;
        last = this_process;
        return;
    }
    Node *curr = head;
    Node *prev = NULL;
    switch(sched_type)
    {
        case RR:
            last->next = this_process;
            last = this_process;
            break;
        case STCF:
            if(process->thread_length >= last->data->thread_length)
            {
                last->next = this_process;
                last = this_process;
                break;
            }
            if(process->thread_length < head->data->thread_length)
            {
                this_process->next = head;
                head = this_process;
                break;
            }
            prev = curr;
            curr = curr->next;
            while(curr != NULL)
            {
                if(process->thread_length < curr->data->thread_length)
                {
                    prev->next = this_process;
                    this_process->next = curr;
                    break;
                }
                prev = curr;
                curr = curr->next;
            }

            break;
        case PRR:
            if(process->priority >= last->data->priority)
            {
                last->next = this_process;
                last = this_process;
                break;
            }
            if(process->priority < head->data->priority)
            {
                this_process->next = head;
                head = this_process;
                break;
            }
            prev = curr;
            curr = curr->next;
            while(curr != NULL)
            {
                if(process->priority < curr->data->priority)
                {
                    prev->next = this_process;
                    this_process->next = curr;
                    break;
                }
                prev = curr;
                curr = curr->next;
            }
            break;
        default:
            fprintf(stderr, "\nUnknown scheduling type set. Seting to default (RR) instead.\n");
            sched_type = RR;
            queue_process(process);
    }
}

int main(int argc, char **argv)
{
    DEBUG_PRINT("%s\n", "starting in debug mode...");
    DEBUG_PRINT("Time set to %d. Timeslice set to %d\n", pseudo_clock, TIMESLICE);
    char fbuff[BUFFER_SIZE];
    FILE *fptr;

    /* check for errors in initial setup */
    // check if an argument was passed
    if(argc < 2)
    {
        fprintf(stderr, "[NEED ONE MORE ARGUMENT] Usage: <scheduler#> [file name]\n");
        exit(EXIT_FAILURE);
    }
    DEBUG_PRINT("Running arg(%d): ", argc);
    if(DEBUG_TASK)
    {
        for(int i=0; i < argc-1; i++)
        {
            DEBUG_PRINT("%s, ", argv[i]);
        }
        DEBUG_PRINT("%s\n", argv[argc-1]);
    }
    // open file
    if(argc > 2)
    {
        if((fptr = fopen(argv[2], "r")) == NULL)
        {
            fprintf(stderr, "File \"%s\" not found\n", argv[2]);
            exit(EXIT_FAILURE);
        }
    }
    // use default file if no file is specified
    else
    {
        if((fptr = fopen("processes.txt", "r")) == NULL)
        {
            fprintf(stderr, "Default file \"%s\" not found. Consider adding a file name as the 3rd argument.\n", "processes.txt");
            exit(EXIT_FAILURE);
        }
    }
    // set scheduling type
//    DEBUG_PRINT("argv[1] value: %d, isdigit result: %d\n", *argv[1] - '0', isdigit(*argv[1]));
    if((*argv[1]-'0'  > 0) && (*argv[1]-'0' < SCHEDULE_TYPE_COUNT))
    {
        sched_type = *argv[1] - '0';
    }
    else
    {
        fprintf(stderr, "Unknown schedule type (Use: {1,2,3}). Using default (1) instead\n");
    }
    print_schedule_type();

    // read and queue up processes
    while(1)
    {
        // read a line (process info) from the file
        // break if EOF
        if(fscanf(fptr, "%[^\n]\n", fbuff) == EOF)
        {
            break;
        }
//        DEBUG_PRINT("Line from fscanf: %s\n", fbuff);
        char *token = strtok(fbuff, " ");
        int counter = 0;
        char **fields = malloc(PROCESS_FIELDS * sizeof(char *));
        for(int i=0; i < PROCESS_FIELDS; i++)
        {
            fields[i] = malloc(BUFFER_SIZE * sizeof(char));
        }
        Process *process = malloc(sizeof(Process));


        // parse process info and break it into fields
        while(token)
        {
            strcpy(fields[counter], token);
            counter++;
            token = strtok(NULL, " ");
        }

        // error checking 
//        DEBUG_PRINT("fields populated: %d, required: %d\n", counter, PROCESS_FIELDS);
//        DEBUG_PRINT("\tField: %s, %s, %s, %s, %s\n", fields[0], fields[1], fields[2], fields[3], fields[4]); 
        if(counter != PROCESS_FIELDS)
        {
            fprintf(stderr, "Encountered a non process data. Found %d fields instead of %d\n", counter, PROCESS_FIELDS);
            exit(EXIT_FAILURE);
        }

        // construct the process struct
        process->process_id = process_id++;
        strcpy(process->thread_name, fields[0]);
        process->thread_type = strtol(fields[1], NULL, 10);
        process->priority = strtol(fields[2], NULL, 10);
        process->thread_length = strtol(fields[3], NULL, 10);
        process->odds_of_IO = strtol(fields[4], NULL, 10);
        //TODO: Do we set the time when we receive in queue? or not?
//        process->start = pseudo_clock;
        process->start = -1;
        process->finish = -1;
        process->total = -1;

//        DEBUG_PRINT("%s, %d, %d, %d, %d%%\n", process->thread_name, process->thread_type, process->priority, process->thread_length, process->odds_of_IO);

        queue_process(process);

        // free malloc
//        free(process);
        for(int i=0; i < PROCESS_FIELDS; i++)
        {
            free(fields[i]);
        }
        free(fields);
//        DEBUG_PRINT("%s", "Process queued. Fields freed. Handling next process.\n");
    }
    fclose(fptr);

    DEBUG_PRINT("%s", "Finished reading and queueing up all processes\n");
    if(DEBUG_TASK)
    {
        print_process_queue();
    }

    // finished reading and queueing up the process
    // time to start simulation
    sim_process_queue();

    DEBUG_PRINT("Priority %d time value: %d\n", 0, priority_time[0]);
    DEBUG_PRINT("Priority %d time value: %d\n", 1, priority_time[1]);
    DEBUG_PRINT("Priority %d time value: %d\n", 2, priority_time[2]);

    DEBUG_PRINT("Priority %d counter value: %d\n", 0, priority_counter[0]);
    DEBUG_PRINT("Priority %d counter value: %d\n", 1, priority_counter[1]);
    DEBUG_PRINT("Priority %d counter value: %d\n", 2, priority_counter[2]);

    DEBUG_PRINT("Type %d time value: %d\n", 0, type_time[0]);
    DEBUG_PRINT("Type %d time value: %d\n", 1, type_time[1]);
    DEBUG_PRINT("Type %d time value: %d\n", 2, type_time[2]);
    DEBUG_PRINT("Type %d time value: %d\n", 3, type_time[3]);

    DEBUG_PRINT("Type %d counter value: %d\n", 0, type_counter[0]);
    DEBUG_PRINT("Type %d counter value: %d\n", 1, type_counter[1]);
    DEBUG_PRINT("Type %d counter value: %d\n", 2, type_counter[2]);
    DEBUG_PRINT("Type %d counter value: %d\n", 3, type_counter[3]);

    printf("\nAverage run time per priority:\n");
    for(int i=0; i < NUM_PROCESS_PRIORITY; i++)
    {
        if(priority_counter[i] != 0)
        {
            printf("Priority %d average run time: %d\n", i, priority_time[i]/priority_counter[i]);
        }
        else
        {
            printf("Priority %d average run time: %d\n", i, 0);
        }
    }
    printf("\nAverage run time per type:\n");
    for(int i=0; i < NUM_PROCESS_TYPE; i++)
    {
        if(type_counter[i] != 0)
        {
            printf("Type %d average run time: %d\n", i, type_time[i]/type_counter[i]);
        }
        else
        {
            printf("Type %d average run time: %d\n", i, 0);
        }
    }

    //TODO: program done. free all malloc
}
