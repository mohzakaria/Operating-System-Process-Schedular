#include "headers.h"
#include "string.h"
#include <limits.h>
#include "string.h"
#define MAX 100
#define MAX_SIZE 1024

struct Process
{
    int id;
    int arrival;
    int runtime;
    int priority;
    int memsize;
};

/////////////////////////////////////////////////////////////TREE///////////////////////////////////////////////////////////////////////////////////////

// Node structure for the binary tree
struct node
{
    int size;
    int is_free;
    int is_FirstTime;
    struct node *left;
    struct node *right;
    struct Process process;
    int StartLocation;
};

typedef struct node node_t;

// function to create a new node
node_t *create_node(int size, int startLoc)
{
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    new_node->size = size;
    new_node->is_free = 1;
    new_node->is_FirstTime = 1;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->StartLocation = startLoc;
    return new_node;
}

//  function to find the nearest power of 2 greater than or equal to a given number
int nearest_power_of_2(int num)
{
    int power = 0;
    int res = pow(2, power);
    while (res < num)
    {
        power++;
        res = pow(2, power);
    }
    return res;
}

// function to split a node into two buddies
void split_node(node_t *node, int size)
{
    node->is_free = 0;
    if (node->is_FirstTime == 1)
    {
        node->left = create_node(size, node->StartLocation);
        node->right = create_node(size, (node->StartLocation + size));
    }
}

// function to merge two free buddies
node_t *merge_buddies(node_t *node)
{
    if ((node->left != NULL) && (node->right != NULL))
    {
        if (node->left->is_free && node->right->is_free)
        {
            node_t *left = node->left;
            node_t *right = node->right;
            free(left);
            free(right);
            node->left = NULL;
            node->right = NULL;
            node->is_free = 1;
            node->is_FirstTime = 1;
            return node;
        }
    }
    return NULL;
}

// This function will allocate the a new node of the desired 2^i location
/*
                    1024
                   /      \
                512       512
              /   \      /    \
             256  256    256   256
        add so on
*/
// Recursive function to allocate memory
node_t *allocate(node_t *node, int size)
{
    // printf("\nNode: %d\n", node->size);
    if (node->size != 1024)
    {
        // printf("\nNode free status: %d\n", node->is_free);
        if (node->size < size || !node->is_free)
        {
            // printf("\nReturning\n");
            return NULL;
        }
    }

    if (node->size == size && node->is_free)
    {

        node->is_free = 0;

        // printf("\nNode updated free status: %d\n", node->is_free);
        return node;
    }

    split_node(node, (node->size) / 2);
    node->is_FirstTime = 0;
    node_t *left = allocate(node->left, size);
    if (left != NULL)
    {
        return left;
    }
    printf("\nNow we will see right\n");
    return allocate(node->right, size);
}

// Function that traverse on the tree to search for a particular element
// VIP Note: The given size is not the size of the process needed to be allocated. It is the size 2^i which is 1024 - 512 - 256 - .... - 8
node_t *Search_For_Value(node_t *node, int size)
{

    if (node == NULL)
    {
        return NULL;
    }

    node_t *left = Search_For_Value(node->left, size);
    if (node->size == size && node->is_free == 1)
    {
        return node;
    }

    if (left == NULL) // This means the node is not found on left subtree
    {
        return Search_For_Value(node->right, size);
    }
    else
    {
        return left;
    }
}

// function to check if a node is a leaf (i.e. has no children)
int is_leaf(node_t *node)
{
    return node->left == NULL && node->right == NULL;
}

// Recursive function to free memory
void deallocate(node_t *node, node_t *to_deallocate)
{
    if (node == NULL || to_deallocate == NULL)
    {
        return;
    }

    if (node == to_deallocate)
    {
        node->is_free = 1;
        node->is_FirstTime = 1;
        merge_buddies(node);
        return;
    }

    if (is_leaf(node))
    {
        return;
    }

    deallocate(node->left, to_deallocate);
    deallocate(node->right, to_deallocate);
    merge_buddies(node);
}

// Helper function to print the binary tree
void print_tree(node_t *node)
{
    if (node == NULL)
    {
        return;
    }

    printf("[%d,%d]", node->size, node->is_free);
    if (!is_leaf(node))
    {
        printf("(");
        print_tree(node->left);
        printf(",");
        print_tree(node->right);
        printf(")");
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PCB // Better to use hash table instead of array
{
    int ID_of_ProcessBuffer;
    int p_id; // id of the created child(process) in scheduler
    int runningtime;
    int waitingtime;
    int remainingtime;
    int ArrivalTime;
    int state; // 1 -> Started   2->Resumed   3->Stopped    4->Finished
    bool FirstTime;
    int FinishTime;
    node_t *ProcessInMemory; // this is a pointer on the location of the process in the memory
    int ProcessSize;
};

/*
    Message buffer
*/
struct msgbuff
{
    struct Process processBuffer;
};

struct MSG_BUFF // this will be used to send/recieve the remaining time of a process
{
    long mtype;
    char mtext[20]; // Remaining time
};

/////////////////////////////////////////////////////////////////
// Priorty Queue

struct PriorityQueue
{
    int ProcessQueue;
    int priority;
} 
pq[100];
int rear = -1;

int pqisEmpty()
{
    if (rear == -1)
        return 1;
    else
        return 0;
}

void insert(int proc, int prio)
{
    rear = rear + 1;
    pq[rear].ProcessQueue = proc;
    pq[rear].priority = prio;
}

int pqpeek(int proc)
{
    int i, p = -12;
    if (!pqisEmpty())
    {
        for (i = 0; i <= rear; i++)
        {
            if (pq[i].priority > p)
            {
                p = pq[i].priority;
                proc = pq[i].ProcessQueue;
            }
        }
    }
    return p;
}

int pqdequeue()
{

    int i, j, p, ele;
    int proc;
    p = pqpeek(proc);
    for (i = 0; i <= rear; i++)
    {
        if (pq[i].priority == p)
        {
            proc = pq[i].ProcessQueue;
            break;
        }
    }
    if (i < rear)
    {
        for (j = i; j < rear; j++)
        {
            pq[j].ProcessQueue = pq[j + 1].ProcessQueue;
            pq[j].priority = pq[j + 1].priority;
        }
    }
    rear = rear - 1;
    return proc;
}

//////////////////////////////////////////////////////////////////
/*
    Implmentation of the queue
*/
// struct Process queue [100];
int queue[100]; // this array will contain the id of the created child(process) in scheduler
int count = 0;
void Queue(int p) // struct Process p)
{
    if (count == 100)
    {
        printf("No more space in queue for RR!");
        return;
    }
    queue[count] = p;
    count++;
}

// struct Process Dequeue(){
int Dequeue()
{
    if (count == 0)
    {
        printf("Error! queue is empty");
        return -1;
    }
    // struct Process pro = queue[0];
    int pro = queue[0];
    for (int i = 0; i < count - 1; i++)
    {
        queue[i] = queue[i + 1];
    }
    count--;
    return pro;
}

int Peek()
{
    // struct Process pro = queue[0];
    int pro = queue[0];
    return pro;
}

int IsEmpty()
{
    if (count == 0)
        return 1;
    else
        return 0;
}

struct Process queueForMemory[100];
int count_memory = 0;
void QueueMemory(struct Process p) // struct Process p)
{
    if (count_memory == 100)
    {
        printf("No more space in memory queue!");
        return;
    }
    queueForMemory[count_memory] = p;
    count_memory++;
}

struct Process Dequeue_Memory()
{
    // check is the queue is empty before calling this function
    //  if (count_memory == 0)
    //  {
    //      printf("Error! queue is empty");
    //      return NULL;
    //  }
    struct Process pro = queueForMemory[0];
    for (int i = 0; i < count_memory - 1; i++)
    {
        queueForMemory[i] = queueForMemory[i + 1];
    }
    count_memory--;
    return pro;
}
struct Process Peek_MemoryQueue()
{
    struct Process pro = queueForMemory[0];
    return pro;
}
int IsEmpty_Memory()
{
    if (count_memory == 0)
        return 1;
    else
        return 0;
}

int val_Sch, msgq_id_Sch, val_Process, msgq_id_Process;
void CreateMessageQueue_sch_process()
{
    key_t key_id;

    /* message up buffer*/

    key_id = ftok("keyfile", 80);                   // create unique key
                                                    // printf("\nkey_id = %d\n", key_id);
    msgq_id_Sch = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id

    if (msgq_id_Sch == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    // printf("\nMessage Scheduler Queue ID = %d\n", msgq_id_Sch);
    /*for message process buffer*/

    key_id = ftok("keyfile", 81); // create unique key
    printf("\nkey_id = %d\n", key_id);
    msgq_id_Process = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id

    if (msgq_id_Process == -1)
    {
        perror("\nError in create\n");
        exit(-1);
    }
    // printf("\nMessage Process Queue ID = %d\n", msgq_id_Process);
}

/*
    Create an array for PCB
*/
struct PCB pcb[100];
int pcb_index = 0;

void PCB_Table(int p_id, struct Process p, node_t *node)
{
    pcb[pcb_index].ID_of_ProcessBuffer = p.id; // id from the iput file
    pcb[pcb_index].p_id = p_id;                // id of the child
    pcb[pcb_index].remainingtime = p.runtime;  // this will be reguarly updated
    pcb[pcb_index].waitingtime = 0;
    pcb[pcb_index].runningtime = p.runtime;
    pcb[pcb_index].ArrivalTime = p.arrival;
    pcb[pcb_index].state = 1; // better to use enum
    pcb[pcb_index].FirstTime = true;
    pcb[pcb_index].ProcessInMemory = node;
    pcb[pcb_index].ProcessSize = p.memsize;
}

bool Running_Status = false;

// Define root for the binary tree that would be used for the memory
node_t *root;

// function that will allocate the process in the memory
node_t *Allocate_Memory(struct Process p);

// function for starting a process
void Add_Process_List(struct Process p, node_t *node, int algo);

int rec_val, msgq_id;
int clk;
struct msgbuff Processes; // processes coming from the process generator
struct MSG_BUFF msg_Sch;
struct MSG_BUFF msg_Process;

bool ProcessFinished = false; // for RR

int TotalRunTime = 0;
int p; // process id

float TotalWTA = 0;

// void handler(int signum);
// int Scheduling_Algo_global;
int main(int argc, char *argv[])
{
    FILE *fp;
    FILE *FMemory;
    FILE *Fpref;
    fp = fopen("scheduler.log", "w");
    fprintf(fp, "#At time x process y state arr w total z remain wait k\n");
    Fpref = fopen("scheduler.pref", "w");
    FMemory = fopen("memory.log", "w");
    fprintf(FMemory, "#At time x allocated y bytes for process z from i to j\n");

    // signal(SIGUSR1, handler);
    /*
        connecting the message queue with the process_generator
    */
    key_t key_id;

    key_id = ftok("keyfile", 65);               // create unique key
    msgq_id = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id

    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    else
    {
        printf("\nScheduler has been connected to the message queue with process_generator with id %d\n", msgq_id);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /*Create to message queues between the scheduler and the process*/

    CreateMessageQueue_sch_process();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /*
        Taking the parameters sent from the process_generator
    */
    int Scheduling_Algo = atoi(argv[1]);
    int quanta = atoi(argv[2]);
    int noProcess = atoi(argv[3]);

    // Scheduling_Algo_global = Scheduling_Algo;
    float arrWTA[noProcess];
    int TotalWaiting = 0;
    int ProccessFinihed = 0;
    // float* arrWTA;
    //     arrWTA = (float*)malloc(1*sizeof(float));

    // Initialize the binary tree with the maximum size
    root = create_node(MAX_SIZE, 0);

    // Initialize of clock
    initClk();
    /*
        Step 1 driving the chosen algorthim
    */
    if (Scheduling_Algo == 1) // HPF
    {
        int RTofRunning = -1;
        int CLKforRunning = 0;
        /*
                Recieve any process from process generator
            */
        while (1)
        {
            clk = getClk();
            if ((Running_Status == true) && (CLKforRunning != clk) && (RTofRunning > 0))
            {
                RTofRunning--;
                CLKforRunning = clk;
            }

            rec_val = 0;
            while (rec_val != -1)
            {
                rec_val = msgrcv(msgq_id, &Processes, sizeof(Processes.processBuffer), 0, IPC_NOWAIT);
                // IPC_NOWAIT: return -1 immediately if the queue is empty and the message can't be received
                // !IPC_NOWAIT : block until the message can be received
                // 0 : ignore
                // MSG_NOERROR : truncate the message if it is too long to fit in the queue (default: return -1 and the msg is not received and stays in queue)
                if (rec_val != -1) // A process has been received
                {
                    printf("At time %d received procees %d\n", clk, Processes.processBuffer.id);
                    /*
                        Fork to create a new process
                    */
                    node_t *allocate = Allocate_Memory(Processes.processBuffer);
                    if (allocate != NULL) // the process has been allocated
                    {
                        printf("process %d has been allocated\n", Processes.processBuffer.id);
                        int pidProcess = fork();
                        if (pidProcess == -1)
                        {
                            perror("error in fork");
                        }
                        if (pidProcess == 0) // child
                        {

                            //  Child id
                            int CP_id = getpid();
                            char id[15];
                            sprintf(id, "%d", CP_id);

                            // parent id
                            int parent_id = getppid();
                            char p_id[15];
                            sprintf(p_id, "%d", parent_id);

                            printf("\nIM CHILD %d\n", CP_id);
                            int TestProcess = execl("./process.out", "process.out", id, p_id, NULL);
                        }
                        else // parent (Scheduler)
                        {
                            /*
                            put the created process in the queue and PCB
                            */
                            kill(pidProcess, SIGSTOP);

                            TotalRunTime = TotalRunTime + Processes.processBuffer.runtime;

                            PCB_Table(pidProcess, Processes.processBuffer, allocate);
                            pcb_index++;

                            // insert received process in the queue
                            insert(pidProcess, (-1 * Processes.processBuffer.priority));

                            // For Memory allocation
                            int SizeofProcess = Processes.processBuffer.memsize;    // This will hold the process size
                            int Process_id = Processes.processBuffer.id;            // This will hold the process id
                            int StartofProcess = allocate->StartLocation;           // This will hold the process start loction in the memory
                            int EndofProcess = StartofProcess + allocate->size - 1; // This will hold the process End loction in the memory
                            printf("\nAt time %d allocated %d bytes from process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                            fprintf(FMemory, "At time %d allocated %d bytes from process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                        }
                    }
                    else
                    {
                        // if the process couldn't be allocated this means that the memory is fully occupied so we will insert it in a queue to wait for a process to finish
                        QueueMemory(Processes.processBuffer);
                    }
                }
            }

            if (!(Running_Status) && !(pqisEmpty())) // No process is currently running
            {
                p = pqdequeue();
                kill(p, SIGCONT);
                Running_Status = true;

                int Req_Index = 0;
                for (int i = 0; i < pcb_index; i++)
                {
                    if (p == pcb[i].p_id) // we found the process id in the PCB
                    {
                        Req_Index = i;
                    }
                }
                RTofRunning = pcb[pcb_index].runningtime;
                CLKforRunning = getClk();
                /*
                    Send the remaining time to the process
                */

                pcb[Req_Index].waitingtime = clk - pcb[Req_Index].ArrivalTime;

                printf("\nAt time %d process %d started arr %d total %d remain %d wait %d \n",
                       clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime, pcb[Req_Index].runningtime, pcb[Req_Index].runningtime, pcb[Req_Index].waitingtime);

                fprintf(fp, "At time %d process %d started arr %d total %d remain %d wait %d \n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime, pcb[Req_Index].runningtime, pcb[Req_Index].runningtime, pcb[Req_Index].waitingtime);

                msg_Process.mtype = p;
                char str[20];
                sprintf(str, "%d", pcb[Req_Index].remainingtime);
                strcpy(msg_Process.mtext, str);

                val_Process = msgsnd(msgq_id_Process, &msg_Process, sizeof(msg_Process.mtext), !IPC_NOWAIT);
                if (val_Process == -1)
                    perror("Error in send from scheduler to the process!\n");
                else
                {
                    // Modify PCB
                    pcb[Req_Index].state = 1;
                }
            }
            if ((RTofRunning == 0) && (Running_Status == true))
            {
                val_Sch = msgrcv(msgq_id_Sch, &msg_Sch, sizeof(msg_Sch.mtext), 0, !IPC_NOWAIT);
                // IPC_NOWAIT: return -1 immediately if the queue is empty and the message can't be received
                if (val_Sch != -1)
                {
                    p = (int)msg_Sch.mtype;
                    int Req_Index = 0;
                    for (int i = 0; i < pcb_index; i++)
                    {
                        if (p == pcb[i].p_id) // we found the process id in the PCB
                        {
                            Req_Index = i;
                        }
                    }

                    clk = getClk();

                    pcb[Req_Index].waitingtime=clk-pcb[Req_Index].ArrivalTime-pcb[Req_Index].runningtime;

                    pcb[Req_Index].FinishTime = clk;
                    int TA = (pcb[Req_Index].FinishTime) - (pcb[Req_Index].ArrivalTime);
                    float WTA = (float)TA / (float)pcb[Req_Index].runningtime;
                    arrWTA[Req_Index] = WTA;
                    TotalWTA = TotalWTA + WTA;
                    TotalWaiting = TotalWaiting + pcb[Req_Index].waitingtime;

                    printf("\nAt time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime,
                           pcb[Req_Index].runningtime, pcb[Req_Index].waitingtime, TA, WTA);

                    fprintf(fp, "At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime, pcb[Req_Index].runningtime, pcb[Req_Index].waitingtime, TA, WTA);

                    int SizeofProcess = pcb[Req_Index].ProcessSize;                               // This will hold the process size
                    int Process_id = pcb[Req_Index].ID_of_ProcessBuffer;                          // This will hold the process id
                    int StartofProcess = pcb[Req_Index].ProcessInMemory->StartLocation;           // This will hold the process start loction in the memory
                    int EndofProcess = StartofProcess + pcb[Req_Index].ProcessInMemory->size - 1; // This will hold the process End loction in the memory
                    printf("\nAt time %d freed %d bytes from process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                    fprintf(FMemory, "At time %d freed %d bytes from process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                    Running_Status = false;
                    ProccessFinihed++;
                    // Remove the process from the memory
                    deallocate(root, pcb[Req_Index].ProcessInMemory);
                    // print_tree(root);

                    //  check if there is a process in the queue for the memory allocation
                    if (!IsEmpty_Memory())
                    {
                        struct Process pr = Peek_MemoryQueue();
                        node_t *allo = Allocate_Memory(pr);
                        if (allo != NULL)
                        {
                            // call funtion that joins this process with the list of the running processes
                            pr = Dequeue_Memory();
                            Add_Process_List(pr, allo, Scheduling_Algo);

                            // For memory allocation
                            int SizeofProcess = pr.memsize; // This will hold the process size
                            int Process_id = pr.id;         // This will hold the process id
                            int StartofProcess = allo->StartLocation;            // This will hold the process start loction in the memory
                            int EndofProcess = StartofProcess + allo->size -1;      // This will hold the process End loction in the memory
                            printf("\nAt time %d allocated %d bytes for process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                            fprintf(FMemory, "At time %d allocated %d bytes for process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                        
                        }
                        // else
                        // {
                        //     QueueMemory(p);
                        // }
                        // struct Process p = Peek_MemoryQueue();
                        // if (p.memsize)
                    }
                }
            }

            if (noProcess == ProccessFinihed) // No process is currently running an all process have finished
            {
                clk = getClk();
                float CPUutilization = (100 * (float)TotalRunTime) / (float)clk;
                float AvgWTA = (float)TotalWTA / (float)noProcess;
                float AvgWaiting = (float)TotalWaiting / (float)noProcess;
                float StdWTA = 0;
                for (int j = 0; j < noProcess; j++)
                {
                    StdWTA = StdWTA + ((arrWTA[j] - AvgWTA) * (arrWTA[j] - AvgWTA));
                }
                StdWTA = StdWTA / noProcess;
                StdWTA = sqrt(StdWTA);
                printf("\nCPU utilization = %.2f%% \nAvg WTA = %.2f \nAvg WAiting = %.2f \nStd WTA = %.2f \n", CPUutilization, AvgWTA, AvgWaiting, StdWTA);
                fprintf(Fpref, "CPU utilization = %.2f%% \nAvg WTA = %.2f \nAvg WAiting = %.2f \nStd WTA = %.2f \n", CPUutilization, AvgWTA, AvgWaiting, StdWTA);
                break;
            }
        }
    }

    else if (Scheduling_Algo == 2) // STRN
    {
        int RTofRunning = -1;
        int CLKforRunning = 0;
        while (1)
        {
            clk = getClk();
            if ((Running_Status == true) && (CLKforRunning != clk) && (RTofRunning > 0))
            {
                RTofRunning--;
                CLKforRunning = clk;
            }
            rec_val = 0;
            while (rec_val != -1)
            {
                // Decrementing RT of current running process when clk changes

                // receive processes from the process generator
                rec_val = msgrcv(msgq_id, &Processes, sizeof(Processes.processBuffer), 0, IPC_NOWAIT);
                // IPC_NOWAIT: return -1 immediately if the queue is empty and the message can't be received
                // !IPC_NOWAIT : block until the message can be received
                // 0 : ignore
                // MSG_NOERROR : truncate the message if it is too long to fit in the queue (default: return -1 and the msg is not received and stays in queue)
                if (rec_val != -1) // A process has been received
                {
                    printf("Scheduler recived process %d at time %d", Processes.processBuffer.id, clk);
                    node_t *allocate = Allocate_Memory(Processes.processBuffer);
                    if (allocate != NULL) // the process has been allocated
                    {
                        /*
                            Fork to create a new process
                        */
                        printf("\nScheduler has recived a process with id %d and priority %d\n", Processes.processBuffer.id, Processes.processBuffer.runtime);

                        // fork to create the processes
                        int pidProcess = fork();
                        if (pidProcess == -1)
                        {
                            perror("error in fork");
                        }
                        if (pidProcess == 0) // child
                        {

                            //  Child id
                            int CP_id = getpid();
                            char id[15];
                            sprintf(id, "%d", CP_id);

                            // parent id
                            int parent_id = getppid();
                            char p_id[15];
                            sprintf(p_id, "%d", parent_id);

                            // give the processes its parameters (child id, parent id)
                            int TestProcess = execl("./process.out", "process.out", id, p_id, NULL);
                        }
                        else // parent (Scheduler)
                        {
                            /*
                            put the created process in the queue and PCB
                            */
                            TotalRunTime = TotalRunTime + Processes.processBuffer.runtime;
                            kill(pidProcess, SIGSTOP);
                            PCB_Table(pidProcess, Processes.processBuffer, allocate);
                            pcb_index++;

                            // For memory allocation
                            int SizeofProcess = Processes.processBuffer.memsize;    // This will hold the process size
                            int Process_id = Processes.processBuffer.id;            // This will hold the process id
                            int StartofProcess = allocate->StartLocation;           // This will hold the process start loction in the memory
                            int EndofProcess = StartofProcess + allocate->size - 1; // This will hold the process End loction in the memory
                            printf("\nAt time %d allocated %d bytes for process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                            fprintf(FMemory, "At time %d allocated %d bytes for process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                            // check the processes runtime with the currently running process
                            /*
                                receive process from the process.c in 2 cases:
                                CASE 1: The process completes its runtime
                                CASE 2: We recieved a process that has runtime < currently running  process
                                if we didnt recieve anything from the process -> loop all over again
                            */
                            if ((RTofRunning > Processes.processBuffer.runtime) && (Running_Status == true))
                            {
                                kill(p, SIGTSTP);

                                val_Sch = msgrcv(msgq_id_Sch, &msg_Sch, sizeof(msg_Sch.mtext), 0, !IPC_NOWAIT);
                                if (val_Sch == -1)
                                {
                                    perror("Process: Error in recive\n");
                                }
                                else
                                {

                                    /*
                                        loop to get the process in the PCB
                                    */
                                    int index_Rec_Process = 0;
                                    for (int i = 0; i < pcb_index; i++)
                                    {
                                        if (msg_Sch.mtype == pcb[i].p_id)
                                        {
                                            index_Rec_Process = i;
                                        }
                                    }

                                    int RT = atoi(msg_Sch.mtext);

                                    /*
                                        insert process in the queue and modify the PCB
                                    */
                                    // else
                                    // {
                                    insert(p, (-1 * RT)); // This will insert the currently running process

                                    pcb[index_Rec_Process].remainingtime = RT;
                                    pcb[index_Rec_Process].state = 3; // stopped
                                    pcb[index_Rec_Process].FirstTime = false;
                                    Running_Status = false;
                                    printf("\nAt time %d process %d stopped arr %d total %d remain %d wait %d\n", clk, pcb[index_Rec_Process].ID_of_ProcessBuffer, pcb[index_Rec_Process].ArrivalTime, pcb[index_Rec_Process].runningtime, RT, pcb[index_Rec_Process].waitingtime);
                                    fprintf(fp, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", clk, pcb[index_Rec_Process].ID_of_ProcessBuffer, pcb[index_Rec_Process].ArrivalTime, pcb[index_Rec_Process].runningtime, RT, pcb[index_Rec_Process].waitingtime);
                                }
                            }

                            // insert the processes in the priority queue based on their runtime
                            insert(pidProcess, (-1 * Processes.processBuffer.runtime)); // This will insert the new process
                        }
                    }
                    else
                    {
                        QueueMemory(Processes.processBuffer);
                    }
                }
            }

            if (!(Running_Status) && !(pqisEmpty()))
            {

                p = pqdequeue();
                Running_Status = true;

                /*
                    Get the requried process from the PCB
                */
                int Req_Index = 0;
                for (int i = 0; i < pcb_index; i++)
                {
                    if (p == pcb[i].p_id) // we found the process id in the PCB
                    {
                        Req_Index = i;
                    }
                }
                /*
                    Send the remaining time to the process
                */
                kill(p, SIGCONT);

                RTofRunning = pcb[Req_Index].remainingtime;
                CLKforRunning = getClk();

                msg_Process.mtype = p;

                /*
                    sending remaining time to process
                */
                char str[20];
                sprintf(str, "%d", pcb[Req_Index].remainingtime);
                strcpy(msg_Process.mtext, str);
                val_Process = msgsnd(msgq_id_Process, &msg_Process, sizeof(msg_Process.mtext), !IPC_NOWAIT);
                if (val_Process == -1)
                {
                    perror("\nError in send from scheduler to the process!\n");
                }
                else
                {
                    //    printf("\nScheduler sent to process with id %d permission with RT = %d at time %d\n", p, atoi(msg_Process.mtext), clk);

                    /*
                        Modify PCB
                    */
                    clk = getClk();
                    if (pcb[Req_Index].FirstTime)
                    {
                        pcb[Req_Index].waitingtime = clk - pcb[Req_Index].ArrivalTime;
                        pcb[Req_Index].state = 1;
                        pcb[Req_Index].FirstTime = false;
                        printf("\nAt time %d process %d started arr %d total %d remain %d wait %d\n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime, pcb[Req_Index].runningtime, pcb[Req_Index].remainingtime, pcb[Req_Index].waitingtime);
                        fprintf(fp, "At time %d process %d started arr %d total %d remain %d wait %d\n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime, pcb[Req_Index].runningtime, pcb[Req_Index].remainingtime, pcb[Req_Index].waitingtime);
                    }
                    else
                    {
                        pcb[Req_Index].waitingtime = clk - pcb[Req_Index].ArrivalTime+pcb[Req_Index].runningtime-pcb[Req_Index].remainingtime;
                        pcb[Req_Index].state = 2;
                        printf("\nAt time %d process %d resumed arr %d total %d remain %d wait %d\n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime, pcb[Req_Index].runningtime, pcb[Req_Index].remainingtime, pcb[Req_Index].waitingtime);
                        fprintf(fp, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime, pcb[Req_Index].runningtime, pcb[Req_Index].remainingtime, pcb[Req_Index].waitingtime);
                    }
                }
            }
            if ((Running_Status == true) && (RTofRunning == 0))
            {
                val_Sch = msgrcv(msgq_id_Sch, &msg_Sch, sizeof(msg_Sch.mtext), 0, !IPC_NOWAIT);
                // IPC_NOWAIT: return -1 immediately if the queue is empty and the message can't be received
                if (val_Sch != -1)
                {
                    p = (int)msg_Sch.mtype;
                    int Req_Index = 0;
                    for (int i = 0; i < pcb_index; i++)
                    {
                        if (p == pcb[i].p_id) // we found the process id in the PCB
                        {
                            Req_Index = i;
                        }
                    }

                    clk = getClk();

                    pcb[Req_Index].waitingtime=clk-pcb[Req_Index].ArrivalTime-pcb[Req_Index].runningtime;

                    pcb[Req_Index].FinishTime = clk;
                    int TA = (pcb[Req_Index].FinishTime) - (pcb[Req_Index].ArrivalTime);
                    float WTA = (float)TA / (float)pcb[Req_Index].runningtime;
                    arrWTA[Req_Index] = WTA;
                    TotalWTA = TotalWTA + WTA;
                    TotalWaiting = TotalWaiting + pcb[Req_Index].waitingtime;

                    printf("\nAt time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime,
                           pcb[Req_Index].runningtime, pcb[Req_Index].waitingtime, TA, WTA);

                    fprintf(fp, "\nAt time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime, pcb[Req_Index].runningtime, pcb[Req_Index].waitingtime, TA, WTA);

                    // For memory allocation
                    int SizeofProcess = pcb[Req_Index].ProcessSize;                               // This will hold the process size
                    int Process_id = pcb[Req_Index].ID_of_ProcessBuffer;                          // This will hold the process id
                    int StartofProcess = pcb[Req_Index].ProcessInMemory->StartLocation;           // This will hold the process start loction in the memory
                    int EndofProcess = StartofProcess + pcb[Req_Index].ProcessInMemory->size - 1; // This will hold the process End loction in the memory
                    printf("\nAt time %d freed %d bytes from process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                    fprintf(FMemory, "At time %d freed %d bytes from process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);

                    // Remove the process from the memory
                    deallocate(root, pcb[Req_Index].ProcessInMemory);

                    //  check if there is a process in the queue for the memory allocation
                    if (!IsEmpty_Memory())
                    {
                        struct Process pr = Peek_MemoryQueue();
                        node_t *allo = Allocate_Memory(pr);
                        if (allo != NULL)
                        {
                            // call funtion that joins this process with the list of the running processes
                            pr = Dequeue_Memory();
                            Add_Process_List(pr, allo, Scheduling_Algo);
                            // For memory allocation
                            int SizeofProcess = pr.memsize; // This will hold the process size
                            int Process_id = pr.id;         // This will hold the process id
                            int StartofProcess = allo->StartLocation;            // This will hold the process start loction in the memory
                            int EndofProcess = StartofProcess + allo->size -1;      // This will hold the process End loction in the memory
                            printf("\nAt time %d allocated %d bytes for process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                            fprintf(FMemory, "At time %d allocated %d bytes for process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                        }
                    }

                    Running_Status = false;
                    ProccessFinihed++;
                }
            }

            if (noProcess == (ProccessFinihed)) // No process is currently running an all process have finished
            {
                clk = getClk();
                float CPUutilization = (100 * (float)TotalRunTime) / (float)clk;
                float AvgWTA = (float)TotalWTA / (float)noProcess;
                float AvgWaiting = (float)TotalWaiting / (float)noProcess;
                float StdWTA = 0;
                for (int j = 0; j < noProcess; j++)
                {
                    StdWTA = StdWTA + ((arrWTA[j] - AvgWTA) * (arrWTA[j] - AvgWTA));
                }
                StdWTA = StdWTA / noProcess;
                StdWTA = sqrt(StdWTA);
                printf("\nCPU utilization = %.2f%% \nAvg WTA = %.2f \nAvg WAiting = %.2f \nStd WTA = %.2f \n", CPUutilization, AvgWTA, AvgWaiting, StdWTA);
                fprintf(Fpref, "CPU utilization = %.2f%% \nAvg WTA = %.2f \nAvg WAiting = %.2f \nStd WTA = %.2f \n", CPUutilization, AvgWTA, AvgWaiting, StdWTA);
                break;
            }
        }
    }

    else if (Scheduling_Algo == 3) // RR
    {
        int Total_RT = 0;
        int RTofRunning = -1;
        int CLKforRunning = 0;
        int id = getpid();
        bool IsFinished = false;
        while (1)
        {
            /*
                Recieve any process from process generator
            */

            clk = getClk();
            rec_val = 0;
            while (rec_val != -1)
            {
                rec_val = msgrcv(msgq_id, &Processes, sizeof(Processes.processBuffer), 0, IPC_NOWAIT);
                // IPC_NOWAIT: return -1 immediately if the queue is empty and the message can't be received
                // !IPC_NOWAIT : block until the message can be received
                // 0 : ignore
                // MSG_NOERROR : truncate the message if it is too long to fit in the queue (default: return -1 and the msg is not received and stays in queue)

                if (rec_val != -1) // A process has been received
                {
                    printf("Scheduler recived process %d in time %d\n", Processes.processBuffer.id, clk);
                    node_t *allocate = Allocate_Memory(Processes.processBuffer);
                    if (allocate != NULL) // the process has been allocated
                    {
                        printf("Process %d has been allocated\n", Processes.processBuffer.id);
                        /*
                            Fork to create a new process
                        */
                        int pidProcess = fork();
                        if (pidProcess == -1)
                        {
                            perror("error in fork");
                        }
                        if (pidProcess == 0) // child
                        {
                            /*
                                Here we will give the process its id and the schdeluer id
                            */
                            // Child id
                            int CP_id = getpid();
                            char id[15];
                            sprintf(id, "%d", CP_id);

                            // parent id
                            int parent_id = getppid();
                            char p_id[15];
                            sprintf(p_id, "%d", parent_id);
                            int TestProcess = execl("./process.out", "process.out", id, p_id, NULL);
                        }
                        else // parent (Scheduler)
                        {
                            /*
                            put the created process in the queue and PCB
                            */
                            TotalRunTime = TotalRunTime + Processes.processBuffer.runtime;
                            kill(pidProcess, SIGSTOP);
                            PCB_Table(pidProcess, Processes.processBuffer, allocate);
                            pcb_index++;
                            Queue(pidProcess); // This id is for the created child

                            // For memory allocation
                            int SizeofProcess = Processes.processBuffer.memsize;    // This will hold the process size
                            int Process_id = Processes.processBuffer.id;            // This will hold the process id
                            int StartofProcess = allocate->StartLocation;           // This will hold the process start loction in the memory
                            int EndofProcess = StartofProcess + allocate->size - 1; // This will hold the process End loction in the memory
                            printf("\nAt time %d allocated %d bytes for process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                            fprintf(FMemory, "At time %d allocated %d bytes for process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                        }
                    }
                    else
                    {
                        // if the process couldn't be allocated this means that the memory is fully occupied so we will insert it in a queue to wait for a process to finish
                        QueueMemory(Processes.processBuffer);
                    }
                }
            }

            /*
                Run the process as their order in the queue
            */
            if (!(Running_Status) && !(IsEmpty())) // No process is currently running
            {

                p = Dequeue();
                Running_Status = true;

                /*
                    Get the requried process from the PCB
                */
                int Req_Index = 0;
                for (int i = 0; i < pcb_index; i++)
                {
                    if (p == pcb[i].p_id) // we found the process id in the PCB
                    {
                        Req_Index = i;
                    }
                }
                /*
                    Send the remaining time to the process
                */
                kill(p, SIGCONT);
                msg_Process.mtype = p;
                CLKforRunning = clk;

                if ((pcb_index - ProccessFinihed) != 1) // This condition is for the case that we only have a running process and no process in the queue
                {
                    if (pcb[Req_Index].remainingtime > quanta)
                    {
                        RTofRunning = quanta;
                    }
                    else
                    {
                        RTofRunning = pcb[Req_Index].remainingtime;
                        printf("\nRTofRunning: %d\n", RTofRunning);
                        IsFinished = true;
                    }
                }
                else
                {
                    RTofRunning = 0;
                }

                /*
                    sending remaining time to process if it is the first time for the process to run
                */
                if (pcb[Req_Index].FirstTime)
                {
                    char str[20];
                    sprintf(str, "%d", pcb[Req_Index].remainingtime);
                    strcpy(msg_Process.mtext, str);
                    val_Process = msgsnd(msgq_id_Process, &msg_Process, sizeof(msg_Process.mtext), !IPC_NOWAIT);
                    if (val_Process == -1)
                    {
                        perror("\nError in send from scheduler to the process!\n");
                    }
                    else
                    {

                        /*
                            Modify PCB
                        */
                        pcb[Req_Index].waitingtime = clk - pcb[Req_Index].ArrivalTime;
                        pcb[Req_Index].state = 1;
                        pcb[Req_Index].FirstTime = false;
                        printf("\nAt time %d process %d started arr %d total %d remain %d wait %d\n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime, pcb[Req_Index].runningtime, pcb[Req_Index].remainingtime, pcb[Req_Index].waitingtime);
                        fprintf(fp, "At time %d process %d started arr %d total %d remain %d wait %d\n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime, pcb[Req_Index].runningtime, pcb[Req_Index].remainingtime, pcb[Req_Index].waitingtime);
                    }
                }
                else
                {
                    pcb[Req_Index].waitingtime = clk - pcb[Req_Index].ArrivalTime+pcb[Req_Index].runningtime-pcb[Req_Index].remainingtime;
                    pcb[Req_Index].state = 2;
                    printf("\nAt time %d process %d resumed arr %d total %d remain %d wait %d\n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime, pcb[Req_Index].runningtime, pcb[Req_Index].remainingtime, pcb[Req_Index].waitingtime);
                    fprintf(fp, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", clk, pcb[Req_Index].ID_of_ProcessBuffer, pcb[Req_Index].ArrivalTime, pcb[Req_Index].runningtime, pcb[Req_Index].remainingtime, pcb[Req_Index].waitingtime);
                }
            }

            // Here we are monitring the process by decrementing the remaining time every second
            if ((Running_Status == true) && (CLKforRunning != clk) && (RTofRunning > 0))
            {
                RTofRunning--;
                CLKforRunning = clk;
            }
            // When the quanta of the process is finished we will send a signal stop to process so that we can recive the remaining time of the process and save it in the PCB
            if ((Running_Status == true) && (RTofRunning == 0) && ((pcb_index - ProccessFinihed) != 1))
            {
                if (!IsFinished)
                {
                    kill(p, SIGTSTP);
                }
                val_Sch = msgrcv(msgq_id_Sch, &msg_Sch, sizeof(msg_Sch.mtext), 0, !IPC_NOWAIT);
                if (val_Sch == -1)
                {
                    perror("\nError in receving from process\n");
                }
                else
                {

                    /*
                        loop to get the process in the PCB
                    */
                    int index_Rec_Process = 0;
                    for (int i = 0; i < pcb_index; i++)
                    {
                        if (msg_Sch.mtype == pcb[i].p_id)
                        {
                            index_Rec_Process = i;
                        }
                    }

                    int RT = atoi(msg_Sch.mtext);
                    if ((IsFinished == true) && (RTofRunning == 0)) // The Process has finished
                    {
                        IsFinished = false;
                        pcb[index_Rec_Process].remainingtime = 0;
                        pcb[index_Rec_Process].state = 4; // finished
                        pcb[index_Rec_Process].FirstTime = false;
                        Running_Status = false;

                        clk = getClk();

                        pcb[index_Rec_Process].waitingtime=clk-pcb[index_Rec_Process].ArrivalTime-pcb[index_Rec_Process].runningtime;

                        pcb[index_Rec_Process].FinishTime = clk;
                        int TA = (pcb[index_Rec_Process].FinishTime) - (pcb[index_Rec_Process].ArrivalTime);
                        float WTA = (float)TA / (float)pcb[index_Rec_Process].runningtime;
                        arrWTA[index_Rec_Process] = WTA;
                        TotalWTA = TotalWTA + WTA;
                        TotalWaiting = TotalWaiting + pcb[index_Rec_Process].waitingtime;

                        printf("\nAt time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", clk, pcb[index_Rec_Process].ID_of_ProcessBuffer, pcb[index_Rec_Process].ArrivalTime,
                               pcb[index_Rec_Process].runningtime, pcb[index_Rec_Process].waitingtime, TA, WTA);

                        fprintf(fp, "At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", clk, pcb[index_Rec_Process].ID_of_ProcessBuffer, pcb[index_Rec_Process].ArrivalTime, pcb[index_Rec_Process].runningtime, pcb[index_Rec_Process].waitingtime, TA, WTA);

                        // For memory allocation
                        int SizeofProcess = pcb[index_Rec_Process].ProcessSize;                               // This will hold the process size
                        int Process_id = pcb[index_Rec_Process].ID_of_ProcessBuffer;                          // This will hold the process id
                        int StartofProcess = pcb[index_Rec_Process].ProcessInMemory->StartLocation;           // This will hold the process start loction in the memory
                        int EndofProcess = StartofProcess + pcb[index_Rec_Process].ProcessInMemory->size - 1; // This will hold the process End loction in the memory
                        printf("\nAt time %d freed %d bytes from process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                        fprintf(FMemory, "At time %d freed %d bytes from process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);

                        // Remove the process from the memory
                        deallocate(root, pcb[index_Rec_Process].ProcessInMemory);

                        //  check if there is a process in the queue for the memory allocation
                        if (!IsEmpty_Memory())
                        {
                            struct Process pr = Peek_MemoryQueue();
                            node_t *allo = Allocate_Memory(pr);
                            if (allo != NULL)
                            {
                                // call funtion that joins this process with the list of the running processes
                                pr = Dequeue_Memory();
                                Add_Process_List(pr, allo, Scheduling_Algo);
                                // For memory allocation
                                int SizeofProcess = pr.memsize; // This will hold the process size
                                int Process_id = pr.id;         // This will hold the process id
                                int StartofProcess = allo->StartLocation;            // This will hold the process start loction in the memory
                                int EndofProcess = StartofProcess + allo->size;      // This will hold the process End loction in the memory
                                printf("\nAt time %d allocated %d bytes for process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                                fprintf(FMemory, "At time %d allocated %d bytes for process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                            }
                        }

                        ProccessFinihed++;
                    }
                    else
                    {
                        /*
                            insert process in the queue and modify the PCB
                        */
                        Queue(p);
                        pcb[index_Rec_Process].remainingtime = RT;
                        pcb[index_Rec_Process].state = 3; // stopped
                        pcb[index_Rec_Process].FirstTime = false;
                        Running_Status = false;
                        printf("\nAt time %d process %d stopped arr %d total %d remain %d wait %d\n", clk, pcb[index_Rec_Process].ID_of_ProcessBuffer, pcb[index_Rec_Process].ArrivalTime, pcb[index_Rec_Process].runningtime, RT, pcb[index_Rec_Process].waitingtime);
                        fprintf(fp, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", clk, pcb[index_Rec_Process].ID_of_ProcessBuffer, pcb[index_Rec_Process].ArrivalTime, pcb[index_Rec_Process].runningtime, RT, pcb[index_Rec_Process].waitingtime);
                    }

                    if (noProcess == ProccessFinihed) // No process is currently running an all process have finished
                    {
                        clk = getClk();
                        float CPUutilization = (100 * (float)TotalRunTime) / (float)clk;
                        float AvgWTA = (float)TotalWTA / (float)noProcess;
                        float AvgWaiting = (float)TotalWaiting / (float)noProcess;
                        float StdWTA = 0;
                        for (int j = 0; j < noProcess; j++)
                        {
                            StdWTA = StdWTA + ((arrWTA[j] - AvgWTA) * (arrWTA[j] - AvgWTA));
                        }
                        StdWTA = StdWTA / noProcess;
                        StdWTA = sqrt(StdWTA);
                        printf("\nCPU utilization = %.2f%% \nAvg WTA = %.2f \nAvg WAiting = %.2f \nStd WTA = %.2f \n", CPUutilization, AvgWTA, AvgWaiting, StdWTA);
                        fprintf(Fpref, "CPU utilization = %.2f%% \nAvg WTA = %.2f \nAvg WAiting = %.2f \nStd WTA = %.2f \n", CPUutilization, AvgWTA, AvgWaiting, StdWTA);
                        break;
                    }
                }
            }
            // This part is for moniting the process in the case that their only a running process and no processes in the queue
            if ((Running_Status == true) && ((pcb_index - ProccessFinihed) == 1) && (!IsFinished) && (RTofRunning == 0))
            {
                CLKforRunning = getClk();
                //  printf("\nna et5na2t Clock: %d\n", clk);
                int Req_Index = 0;

                for (int i = 0; i < pcb_index; i++)
                {
                    if (p == pcb[i].p_id) // we found the process id in the PCB
                    {
                        Req_Index = i;
                    }
                }
                if (pcb[Req_Index].remainingtime > quanta)
                {
                    RTofRunning = quanta;
                    pcb[Req_Index].remainingtime -= quanta;
                }
                else
                {
                    RTofRunning = pcb[Req_Index].remainingtime;
                    pcb[Req_Index].remainingtime = 0;
                    IsFinished = true;
                }
            }
            // Here when the process finishes in the case there is only one process is running and no processes is in the queue we will receive its parameters and save it in the PCB
            if ((Running_Status == true) && (RTofRunning == 0) && IsFinished && ((pcb_index - ProccessFinihed) == 1))
            {
                // kill(p, SIGTSTP);
                val_Sch = msgrcv(msgq_id_Sch, &msg_Sch, sizeof(msg_Sch.mtext), 0, !IPC_NOWAIT);
                if (val_Sch == -1)
                {
                    perror("\nError in receving from process\n");
                }
                else
                {

                    /*
                        loop to get the process in the PCB
                    */
                    int index_Rec_Process = 0;
                    for (int i = 0; i < pcb_index; i++)
                    {
                        if (msg_Sch.mtype == pcb[i].p_id)
                        {
                            index_Rec_Process = i;
                        }
                    }

                    int RT = atoi(msg_Sch.mtext);
                    if ((IsFinished == true) && (RTofRunning == 0)) // The Process has finished
                    {
                        IsFinished = false;
                        pcb[index_Rec_Process].remainingtime = 0;
                        pcb[index_Rec_Process].state = 4; // finished
                        pcb[index_Rec_Process].FirstTime = false;
                        Running_Status = false;

                        pcb[index_Rec_Process].FinishTime = clk;
                        int TA = (pcb[index_Rec_Process].FinishTime) - (pcb[index_Rec_Process].ArrivalTime);
                        float WTA = (float)TA / (float)pcb[index_Rec_Process].runningtime;
                        arrWTA[index_Rec_Process] = WTA;
                        TotalWTA = TotalWTA + WTA;
                        TotalWaiting = TotalWaiting + pcb[index_Rec_Process].waitingtime;

                        printf("\nAt time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", clk, pcb[index_Rec_Process].ID_of_ProcessBuffer, pcb[index_Rec_Process].ArrivalTime,
                               pcb[index_Rec_Process].runningtime, pcb[index_Rec_Process].waitingtime, TA, WTA);

                        fprintf(fp, "At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", clk, pcb[index_Rec_Process].ID_of_ProcessBuffer, pcb[index_Rec_Process].ArrivalTime, pcb[index_Rec_Process].runningtime, pcb[index_Rec_Process].waitingtime, TA, WTA);

                        // For memory allocation
                        int SizeofProcess = pcb[index_Rec_Process].ProcessSize;                               // This will hold the process size
                        int Process_id = pcb[index_Rec_Process].ID_of_ProcessBuffer;                          // This will hold the process id
                        int StartofProcess = pcb[index_Rec_Process].ProcessInMemory->StartLocation;           // This will hold the process start loction in the memory
                        int EndofProcess = StartofProcess + pcb[index_Rec_Process].ProcessInMemory->size - 1; // This will hold the process End loction in the memory
                        printf("\nAt time %d freed %d bytes from process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                        fprintf(FMemory, "At time %d freed %d bytes from process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);

                        // Remove the process from the memory
                        deallocate(root, pcb[index_Rec_Process].ProcessInMemory);

                        //  check if there is a process in the queue for the memory allocation
                        if (!IsEmpty_Memory())
                        {
                            struct Process pr = Peek_MemoryQueue();
                            node_t *allo = Allocate_Memory(pr);
                            if (allo != NULL)
                            {
                                // call funtion that joins this process with the list of the running processes
                                pr = Dequeue_Memory();
                                Add_Process_List(pr, allo, Scheduling_Algo);
                                // For memory allocation
                                int SizeofProcess = pr.memsize; // This will hold the process size
                                int Process_id = pr.id;         // This will hold the process id
                                int StartofProcess = allo->StartLocation;            // This will hold the process start loction in the memory
                                int EndofProcess = StartofProcess + allo->size - 1;  // This will hold the process End loction in the memory
                                printf("\nAt time %d allocated %d bytes for process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                                fprintf(FMemory, "At time %d allocated %d bytes for process %d from %d to %d\n", clk, SizeofProcess, Process_id, StartofProcess, EndofProcess);
                            }
                        }

                        ProccessFinihed++;
                    }
                    if (noProcess == ProccessFinihed) // No process is currently running an all process have finished
                    {
                        clk = getClk();
                        float CPUutilization = (100 * (float)TotalRunTime) / (float)clk;
                        float AvgWTA = (float)TotalWTA / (float)noProcess;
                        float AvgWaiting = (float)TotalWaiting / (float)noProcess;
                        float StdWTA = 0;
                        for (int j = 0; j < noProcess; j++)
                        {
                            StdWTA = StdWTA + ((arrWTA[j] - AvgWTA) * (arrWTA[j] - AvgWTA));
                        }
                        StdWTA = StdWTA / noProcess;
                        StdWTA = sqrt(StdWTA);
                        printf("\nCPU utilization = %.2f%% \nAvg WTA = %.2f \nAvg WAiting = %.2f \nStd WTA = %.2f \n", CPUutilization, AvgWTA, AvgWaiting, StdWTA);
                        fprintf(Fpref, "CPU utilization = %.2f%% \nAvg WTA = %.2f \nAvg WAiting = %.2f \nStd WTA = %.2f \n", CPUutilization, AvgWTA, AvgWaiting, StdWTA);
                        break;
                    }
                }
            }
        }
    }
    // upon termination release the clock resources.
    destroyClk(false);
    msgctl(msgq_id_Process, IPC_RMID, (struct msqid_ds *)0);
    msgctl(msgq_id_Sch, IPC_RMID, (struct msqid_ds *)0);
}

// This function will allocate the process in the memory and will return true if succeeded to do so
node_t *Allocate_Memory(struct Process p)
{
    /*
       Steps of implementation:
       1) call function nearest_power_2() and give it the size of the required process
       2) call function Search_For_Value() and give it the number returned from step 1
           -if a node is returned congratulations allocate your process
           -if a NULL is returned go to step 3, then edit it
       3) call funtion allocate() and give it the number returned from step 1
   */

    // 1)
    int n = nearest_power_of_2(p.memsize);
    printf("\nNearst = %d\n", n);
    // 2)
    node_t *loc = Search_For_Value(root, n);
    if (loc != NULL)
    {
        printf("\nthe location of the node is being returned with size %d\n", loc->size);
        loc->is_free = 0;
        loc->process = p; // process has been allocated
        print_tree(root);
        return loc;
    }
    else // no node of such a size
    {
        // 3)
        loc = allocate(root, n);
        if (loc == NULL)
        {
            return NULL;
        }
        loc->process = p;
        // print_tree(root);
        printf("\nthe location of the node is being returned with size %d\n", loc->size);
        return loc;
    }
}

// This function will add a process to the list of process after it has been allocated in the memory
void Add_Process_List(struct Process p, node_t *node, int algo)
{

    int pidProcess = fork();
    if (pidProcess == -1)
    {
        perror("error in fork");
    }
    if (pidProcess == 0) // child
    {

        //  Child id
        int CP_id = getpid();
        char id[15];
        sprintf(id, "%d", CP_id);

        // parent id
        int parent_id = getppid();
        char p_id[15];
        sprintf(p_id, "%d", parent_id);

        printf("\nIM CHILD %d\n", CP_id);
        int TestProcess = execl("./process.out", "process.out", id, p_id, NULL);
    }
    else // parent (Scheduler)
    {
        /*
        put the created process in the queue and PCB
        */
        kill(pidProcess, SIGSTOP);

        TotalRunTime = TotalRunTime + p.runtime;

        PCB_Table(pidProcess, p, node);
        pcb_index++;

        if (algo == 3)
        {
            Queue(pidProcess);
        }
        else if (algo == 1)
        {
            // insert received process in the queue
            insert(pidProcess, (-1 * p.priority));
        }
        else
        {
            insert(pidProcess, (-1 * p.runtime));
        }
    }
    return;
}
