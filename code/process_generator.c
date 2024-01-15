#include "headers.h"
// #include "PriorityQueue.h"
// #include "Queue.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void clearResources(int);

/*
    Process data
*/
struct Process
{
    int id;
    int arrival;
    int runtime;
    int priority;
    int memsize;
};

/*
    Message buffer
*/
struct msgbuff
{
    struct Process processBuffer;
};

/*
    Global Variable so handler can see msgq_id
*/
key_t key_id;
int msgq_id, send_val;

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    /*
        reading the input file
    */
    FILE *fptr;
    fptr = fopen(argv[1], "r");

    /*
        checking for the file
    */
    if (!fptr)
    {
        printf("\nCouldn't open the input file for reading\n");
        return 0;
    }

    /*
       putting the input in an array
   */
    int firstcolumn;
    int secondcolumn;
    int thirdcolumn;
    int fourthcolumn;
    int fifthcolumn;
    int no_process = 0;
    char burn1;
    char burn2;
    char burn3;
    char burn4;
    char burn5;
    fscanf(fptr, "%s %s %s %s %s", &burn1, &burn2, &burn3, &burn4 ,&burn5);

    struct Process P[100];
    while (fscanf(fptr, "%d\t%d\t%d\t%d\t%d", &firstcolumn, &secondcolumn, &thirdcolumn, &fourthcolumn, &fifthcolumn) == 5)
    {
        P[no_process].id = firstcolumn;
        P[no_process].arrival = secondcolumn;
        P[no_process].runtime = thirdcolumn;
        P[no_process].priority = fourthcolumn;
        P[no_process].memsize = fifthcolumn;
        // printf("id:%d \n", P[no_process].id);
        no_process++;
    }
    fclose(fptr);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    /*
        Knowning the scheduling algorithm
    */
    int SchedulingAlgorithm = 0;
    int quanta = 0;
    printf("\nPlease Enter the chosen scheduling algorithm (1 for [HPF] , 2 for [SRTN] , 3 for [RR]): \n");
    scanf("%d", &SchedulingAlgorithm);
    while ((SchedulingAlgorithm != 1) && (SchedulingAlgorithm != 2) && (SchedulingAlgorithm != 3))
    {
        printf("\nError! Please Enter the chosen scheduling algorithm (1 for [HPF] , 2 for [SRTN] , 3 for [RR]): \n");
        scanf("%d", &SchedulingAlgorithm);
    }
    if (SchedulingAlgorithm == 3)
    {
        printf("\nEnter quanta:\n");
        scanf("%d", &quanta);
        while (quanta < 1)
        {
            printf("\nError! please enter a valid quanta (>0): \n");
            scanf("%d", &quanta);
        }
    }
    char type[4];
    sprintf(type, "%d", SchedulingAlgorithm);
    char time[4];
    sprintf(time, "%d", quanta);
    char noProc[4];
    sprintf(noProc, "%d", no_process);

    /*
        Done Knowing
    */

    // 3. Initiate and create the scheduler and clock processes.
    int pidSchedulingAlgorithm;
    int pidClock = fork();
    if (pidClock == -1)
    {
        perror("error in fork");
    }
    if (pidClock == 0)
    {
        int TestClock = execl("./clk.out", "clk.out", NULL);

        if (TestClock < 0)
        {
            printf("\nerror\n");
        }
        return 0;
    }
    else
    {
        // id_clock = pidClock;
        pidSchedulingAlgorithm = fork();
        if (pidSchedulingAlgorithm < 0)
        {
            perror("error in fork");
        }
        if (pidSchedulingAlgorithm == 0)
        {
            int TestSchedulingAlgorithm = execl("./scheduler.out", "scheduler.out", type, time, noProc, NULL);
            if (TestSchedulingAlgorithm < 0)
            {
                printf("\nerror in scheduler child\n");
            }
        }
    }

    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    // int x = getClk();

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.

    // struct PriorityQueue* arr1 = createPriorityQueue(5);
    // peek(arr1);

    // 6. Send the information to the scheduler at the appropriate time.
    key_id = ftok("keyfile", 65);               // create unique key
    msgq_id = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id

    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    // printf("Message ,server, Queue ID up = %d\n", msgq_id);

    int clk = getClk();
    printf("\ncurrent time is %d\n", clk);
    int i = 0;
    printf("\nNo. of process = %d\n", no_process);
    struct msgbuff message;
    kill(pidClock, SIGCONT);
    while (i < no_process)
    { 
        clk = getClk();

        while (clk == P[i].arrival)
        {   
            printf("process id %d arr %d clk %d\n",P[i].id, P[i].arrival, clk);
            message.processBuffer = P[i];
            send_val = msgsnd(msgq_id, &message, sizeof(message.processBuffer), !IPC_NOWAIT);
            // IPC_NOWAIT: return -1 immediately if the queue is full and the message can't be sent
            // !IPC_NOWAIT : block until the message can be sent
            // 0 : ignore
            // MSG_NOERROR : truncate the message if it is too long to fit in the queue (default: return -1 and the msg is not sent and stays in queue)
            if (send_val == -1)
                perror("Errror in send");


            i++;
        }
    }

    // wait for scheduler
    int sid, stat_loc;
    sid = wait(&stat_loc);
    if (!(stat_loc & 0x00FF))
        printf("\nThe Scheduler has been terminated!\n");

    msgctl(msgq_id, IPC_RMID, (NULL)); // removes the queue from the kernel

    // 7. Clear clock resources
    destroyClk(true);
    // printf("\nIm the process generator sending a signal to clock\n");
    // kill(id_clock, SIGINT);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    msgctl(msgq_id, IPC_RMID, (NULL)); // removes the queue from the kernel
    exit(0);
}