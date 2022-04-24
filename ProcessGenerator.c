#include "headers.h"

void handler ( int signum );

int main(int argc, char *argv[])
{
    //assigning a handler to clear ipc resources at termination
    signal (SIGINT, handler);

    // Reading process information from file & entering into an array based on arrival time
    char *filename = argv[1];
    // opening the file
    FILE *fp = fopen(filename, "r");
    int filechar = 0;
    int flag = fscanf(fp, "%d", &filechar);

    // creating the array which will hold the processes
    // to avoid having to put a limit we can just use a queue instead of an array ?????
    struct process ProcessArray[300];
    int index = 0;

    while (flag != EOF)
    {
        // skipping the comment line(s)
        if (flag == 0) // 0 indicates unsuccessful conversion which means that the filechar couldnt read an integer, so the line is a comment.
            fscanf(fp, "%*[^\n]\n");
        else
        {
            ProcessArray[index].id = filechar;
            fscanf(fp, "%d", &filechar);
            ProcessArray[index].arrvialtime = filechar;
            fscanf(fp, "%d", &filechar);
            ProcessArray[index].runtime = filechar;
            fscanf(fp, "%d", &filechar);
            ProcessArray[index].priority = filechar;

            index++;
        }

        flag = fscanf(fp, "%d", &filechar);
    }

    //getting the chosen scheduling algorithm & parameters from the user
    printf("Please choose the scheduling algorithm you want to run:\nEnter 1 for SJF\nEnter 2 for HPF\nEnter 3 for RR\nEnter 4 for Multilevel feedback loop\n");
    scanf( "%s" , argv[1] );
    //in case of RR get the quantum
    if ( atoi(argv[1]) == 3 )
    {
        printf("Please enter the quantum: ");
        scanf( "%s" , argv[2] );
    }
        

    // fork & execute clock
    int clk = fork();
    if ( clk == 0 )
    {
        int execute = execv("./clk", argv);
        // error handling
        if (execute == -1)
            printf("failed to execute clk\n");
        perror("The error is: \n");
        exit(-1);
    }

    // establishing communication with the clock module
    initClk();

    // fork & execute scheduler
    int scheduler = fork();
    if ( scheduler == 0 )
    {
        int execute = execv("./scheduler", argv);
        // error handling
        if (execute == -1)
            printf("failed to execute scheduler\n");
        perror("The error is: \n");
        exit(-1);
    }

    // creating a message queue
    int keyid = ftok(".", 65);
    int msqid = msgget(keyid, 0666 | IPC_CREAT);
    // creating the message
    struct msgbuff message;
    message.mtype = 1; // any arbitary number to send and receive on
    int i=0;

    // loop and send the arrived processes to the scheduler using message queue
    while ( i < index )
    {
        message.process = ProcessArray[i];

        while ( getClk() != message.process.arrvialtime )
        {
            sleep( message.process.arrvialtime - getClk() );
        }

        int send_val = msgsnd(msqid, &message, sizeof(message.process), !IPC_NOWAIT);

        if (send_val == -1)
        {
            perror("Failed to send. The error is ");
        }

        i++;

    }

    //waiting for scheduler to end (end of simulation)
    int status;
    waitpid(scheduler,&status,0);
    
    if (WEXITSTATUS(status)==-1)
        printf("Error in Scheduler\n");


    // releasing communication with the clock module
     destroyClk(1);

    return 0;
}

// Freeing the ipc resources at termination
void handler ( int signum )
{
    //getting the queue ID
    int keyid = ftok(".", 65);
    int msqid = msgget(keyid, 0666 | IPC_CREAT);

    struct msqid_ds * buf;
    //deleting message queue
    msgctl ( msqid , IPC_RMID , buf );

    struct shmid_ds * shmbuf;
    //deleting shared memory
    int shmid = shmget(SHKEY, 4, IPC_CREAT | 0644);
    shmctl ( shmid , IPC_RMID , shmbuf);
    
    exit(1);
}