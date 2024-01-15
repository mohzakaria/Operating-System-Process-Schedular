#include "headers.h"
#include "string.h"

struct MSG_BUFF // this will be used to send/recieve the remaining time of a process
{
    long mtype;
    char mtext[20]; // Remaining time
};

/*
    Creating the message queues between the process and the scheduler
*/
int val_Sch, msgq_id_Sch, val_Process, msgq_id_Process;
void CreateMessageQueue_sch_process()
{
    key_t key_id; //_up;

    /* message up buffer*/

    key_id /*_up*/ = ftok("keyfile", 80); // create unique key
    // printf("key_id = %d\n", key_id);
    msgq_id_Sch = msgget(key_id /*_up*/, 0666 | IPC_CREAT); // create message queue and return id

    if (msgq_id_Sch == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    // printf("Process: Message Scheduler Queue ID = %d\n", msgq_id_Sch);
    /*for message process buffer*/

    key_id /*_down*/ = ftok("keyfile", 81); // create unique key
    // printf("key_id = %d\n", key_id);
    msgq_id_Process = msgget(key_id /*_down*/, 0666 | IPC_CREAT); // create message queue and return id

    if (msgq_id_Process == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    //  printf("Process: Message Process Queue ID = %d\n", msgq_id_Process);
}

struct MSG_BUFF msg_Sch;
struct MSG_BUFF msg_Process;

/*
    Handler for the signal sent from the scheduler
*/
void handler(int signum);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Modify this file as needed*/
int remainingtime;
int My_ID;
int main(int agrc, char *argv[])
{
    CreateMessageQueue_sch_process();
    signal(SIGTSTP, handler);
    initClk();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO it needs to get the remaining time from somewhere
    My_ID = atoi(argv[1]);
    int My_parentID = atoi(argv[2]);
    val_Process = msgrcv(msgq_id_Process, &msg_Process, sizeof(msg_Process.mtext), My_ID, !IPC_NOWAIT);
    if (val_Process == -1)
        perror("Error in receive\n");
    else
    {
        remainingtime = atoi(msg_Process.mtext);
        int counter = 0;
        while (remainingtime > 0)
        {
            counter++;
            remainingtime -= 1;
            sleep(1);
        }
        
        
        msg_Sch.mtype = My_ID;
        char str[20];
        sprintf(str, "%d", remainingtime);
        strcpy(msg_Sch.mtext, str);
       // kill(My_parentID, SIGUSR1);
        val_Sch = msgsnd(msgq_id_Sch, &msg_Sch, sizeof(msg_Sch.mtext), !IPC_NOWAIT);
        if (val_Sch == -1)
            perror("Error in send from scheduler to the process!\n");
        destroyClk(false);
        return 0;
    }
}
void handler(int signum)
{
    /*
        send to the scheduler the remaining time
    */
    char str[20];
    msg_Sch.mtype = My_ID;
    // msg_Sch.mtext = remainingtime;
    sprintf(str, "%d", remainingtime);
    strcpy(msg_Sch.mtext, str);
    val_Sch = msgsnd(msgq_id_Sch, &msg_Sch, sizeof(msg_Sch.mtext), !IPC_NOWAIT);
    if (val_Sch == -1)
    {
        perror("Process: Error in send\n");
    }
    else
    {
        raise(SIGSTOP); // wait until the scheduler send sigcont
    }
}