#include "PriorityQueue.c"

//HPF variables
struct PQNode *Running;
int process_count;

//file variables
FILE *logfile;
FILE *perf;
float utilization=0, avg_wait=0, wta=0;

void ProcessTerminated(int signum);

int main(int argc, char *argv[])
{
    // establishing communication with the clock module
    initClk();

    printf("Sheduler started\n");
    // creating the message queue
    int keyid = ftok(".", 65);
    int msqid = msgget(keyid, 0666 | IPC_CREAT);

    //open the output file
    logfile = fopen("./scheduler.log", "w");
    perf = fopen("./scheduler.perf", "w");

    //printing the header of output file
    fprintf(logfile,"#At time x process y state arr w total z remain y wait k\n");

    //run the chosen algorithm

    ///////////////////////////////// SJF
    if (atoi(argv[1]) == 1)
    {
        signal(SIGUSR1, ProcessTerminated);
        process_count = atoi(argv[3]); // number of processes
        //process_count = 3;
        //struct proc ReadyQ[20] = {0};
        struct PQNode *ReadyQ = NULL;
        Running = NULL;
        int last_runclk;

        while(process_count > 0)
        {
            
            struct msgbuff message;
            int rec_value = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
            while (rec_value != -1)
            {
                //printf("received: %d at time %d \n", message.process.id, getClk());
               
                utilization+=message.process.runtime;
                
                struct PQNode * newnode = PQnewNode(&message.process, message.process.priority, -1,message.process.runtime,0,arrived);

                if (PQisEmpty(&ReadyQ))
                {
                    ReadyQ = newnode;
                }
                else
                {
                    Enqueue_RT(&ReadyQ, newnode);
                }
                
                rec_value = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
            }
            int flag=0;
            if (PQisEmpty(&Running) && !PQisEmpty(&ReadyQ))
            {
                Running = PQDeQueue(&ReadyQ);
               flag=1;
            }
            if (flag==1 && !PQisEmpty(&Running))
            {
                if (Running->processPID == -1)
                {
                    // fork and take id of process
                    int pid = fork();
                    if (pid == 0)
                    {
                        char* runtime_char = malloc(sizeof(char));
                        sprintf(runtime_char, "%d", Running->process.runtime);
                        char *arg[] = {runtime_char, NULL};
                        int execute = execv("./process.out", arg);
                        if (execute == -1)
                            printf("failed to execute process\n");
                        perror("The error is: \n");
                        exit(-1);
                    }
                    Running->processPID = pid;
                    Running->status=started;
                    Running->WaitingTime=(getClk()-Running->process.arrvialtime)-(Running->process.runtime-Running->ReaminingTime);
                    fprintf(logfile,"At time %d process %d started arr %d total %d remain %d wait %d\n",getClk(),Running->process.id,Running->process.arrvialtime,Running->process.runtime,Running->ReaminingTime,Running->WaitingTime);
                }
                
            }

        }
        wta/=atoi(argv[3]);
        avg_wait/=atoi(argv[3]);
        utilization/=getClk();
        utilization*=100;
    }
    ///////////////////////////////// HPF
    else if (atoi(argv[1]) == 2)
    { 
        signal(SIGUSR1, ProcessTerminated);
        process_count = atoi(argv[3]); // number of processes
        struct PQNode *ReadyQ = NULL;
        Running = NULL;
        int last_runclk;

        while (process_count > 0)
        {
            struct msgbuff message;
            int rec_value = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
            while (rec_value != -1)
            {

               // printf("received: %d at time %d \n", message.process.id, getClk());
                utilization+=message.process.runtime;
                
                struct PQNode * newnode = PQnewNode(&message.process, message.process.priority, -1,message.process.runtime,0,arrived);

                if (PQisEmpty(&ReadyQ))
                {
                    ReadyQ = newnode;
                }
                else
                {
                    PQEnQueue(&ReadyQ, newnode);
                }
                //printf("head %d\n", ReadyQ->process.id);
                // if (Running != NULL)
                // {
                //     printf("running %d\n", Running->process.id);
                //     printf("checking condition %d\n", peek(&ReadyQ).priority < Running->process.priority);
                // }
                rec_value = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
            }
            //  peek and dequeue if time or if priority < priority running
            //  in which case enqueue the running
            int flag=0;
            if (PQisEmpty(&Running) && !PQisEmpty(&ReadyQ))
            {
                // running the process
                //printf("before dequeuing %d\n", ReadyQ->process.id);
                Running = PQDeQueue(&ReadyQ);
                //printf("after dequeuing running is empty %d %d is empty q %d \n", isEmpty(&Running), Running->process.id, isEmpty(&ReadyQ));
                flag=1;
            }
            else if (!PQisEmpty(&Running) && !PQisEmpty(&ReadyQ) && (PQpeek(&ReadyQ).priority < Running->process.priority))
            {
                // stopping the old process
                //printf("at stop signal running pid=%d head pid=%d \n", Running->processPID, peek(&ReadyQ).id);
                int iskill = kill(Running->processPID, SIGSTOP);
                Running->ReaminingTime+=last_runclk-getClk();
                fprintf(logfile,"At time %d process %d stopped arr %d total %d remain %d wait %d\n",getClk(),Running->process.id,Running->process.arrvialtime,Running->process.runtime,Running->ReaminingTime,Running->WaitingTime);
                //printf("isstopped %d %d \n", iskill, Running->processPID);
                Running->status=stopped;
                PQEnQueue(&ReadyQ, Running);
                Running = PQDeQueue(&ReadyQ);
                flag=1;
            }

            if (flag==1){
                if (Running->processPID == -1)
                {
                    // fork and take id of process
                    int pid = fork();
                    if (pid == 0)
                    {
                        char* runtime_char = malloc(sizeof(char));
                        sprintf(runtime_char, "%d", Running->process.runtime);
                        //printf("runtime %s of process %d time rn %d\n",runtime_char,getpid(),getClk());
                        char *arg[] = {runtime_char, NULL};
                        int execute = execv("./process.out", arg);
                        // error handling
                        if (execute == -1)
                            printf("failed to execute process\n");
                        perror("The error is: \n");
                        exit(-1);
                    }
                    Running->processPID = pid;
                    Running->status=started;
                    last_runclk=getClk();
                    Running->WaitingTime=(getClk()-Running->process.arrvialtime)-(Running->process.runtime-Running->ReaminingTime);
                    fprintf(logfile,"At time %d process %d started arr %d total %d remain %d wait %d\n",getClk(),Running->process.id,Running->process.arrvialtime,Running->process.runtime,Running->ReaminingTime,Running->WaitingTime);
                }
                else
                {
                   // printf("continuing signal %d\n", Running->processPID);
                    kill(Running->processPID, SIGCONT);
                    Running->status=resumed;
                    last_runclk=getClk();
                   // printf("resumed remaining time %d\n",Running->ReaminingTime);
                    //waiting time = (now-arrival time) - (runtime-remaining time) [total time in system - total time ive run] 
                    Running->WaitingTime=(getClk()-Running->process.arrvialtime)-(Running->process.runtime-Running->ReaminingTime);
                    fprintf(logfile,"At time %d process %d resumed arr %d total %d remain %d wait %d\n",getClk(),Running->process.id,Running->process.arrvialtime,Running->process.runtime,Running->ReaminingTime,Running->WaitingTime);

                }
            }
        }
        //.perf file calculations
        wta/=atoi(argv[3]);
        avg_wait/=atoi(argv[3]);
        utilization/=getClk();
        utilization*=100;
    }
    ///////////////////////////////// RR
    else if (atoi(argv[1]) == 3)
    {
    }
    ///////////////////////////////// MLFL
    else
    {
    }

    printf("exiting scheduler at time %d\n", getClk());

    //printing to .perf file
    fprintf(perf,"CPU utilization= %.2f %% Avg \nWTA =%.2f \nAvg Waiting =%.2f\n",utilization,wta,avg_wait);

    fclose(perf);
    fclose(logfile);

    // releasing communication with the clock module
    destroyClk(1);

    return 0;
}

void ProcessTerminated(int signum)
{
    //.perf file calculations
    avg_wait+=Running->WaitingTime;
    wta+=(float)(getClk()-Running->process.arrvialtime)/Running->process.runtime;
    fprintf(logfile,"At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n",getClk(),Running->process.id,Running->process.arrvialtime,Running->process.runtime,Running->WaitingTime,getClk()-Running->process.arrvialtime,(float)(getClk()-Running->process.arrvialtime)/Running->process.runtime);
    free(Running);
    Running = NULL;
    process_count--;
    //printf("process_count=%d\n", process_count);

}
