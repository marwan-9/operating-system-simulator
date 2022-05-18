#include "PriorityQueue.c"
#include "LinkedList.c"
#include "queue.c"
#include "Tree.c"

//Memory head node
struct Tnode* memory;

// HPF variables
struct PQNode *Running;
int process_count;

// RR variables
int quantum;
int count = 0;

// file variables
FILE *logfile;
FILE *perf;
FILE*memoryfile;
float utilization = 0, avg_wait = 0, wta = 0;

void ProcessTerminated(int signum);
void clockchange(int signum);

struct Tnode* allocateprocess(struct process process)
{
    int start;
    struct Tnode* allocatednode=Allocate(&memory,process.memorysize,&start);
    fprintf(memoryfile,"At time %d allocated %d bytes for process %d from %d to %d\n",getClk(),process.memorysize,process.id,allocatednode->start,allocatednode->end);
    return allocatednode;
}

void deallocateprocess(struct process process,int start,int end)
{
    deallocation(&memory,start,end);
    fprintf(memoryfile,"At time %d freed %d bytes from process %d from %d to %d\n",getClk(),process.memorysize,process.id,start,end);
}

int main(int argc, char *argv[])
{
    //initializing memory
    memory=initMemory();

    // establishing communication with the clock module
    initClk();

    printf("Sheduler started\n");
    // creating the message queue
    int keyid = ftok(".", 65);
    int msqid = msgget(keyid, 0666 | IPC_CREAT);

    // open the output file
    logfile = fopen("./scheduler.log", "w");
    perf = fopen("./scheduler.perf", "w");
    memoryfile=fopen("./memory.log","w");

    // printing the header of output file
    fprintf(logfile, "#At time x process y state arr w total z remain y wait k\n");
    fprintf(memoryfile,"#At time x allocated y bytes for process z from i to j\n");

    // run the chosen algorithm

    ///////////////////////////////// SJF
    if (atoi(argv[1]) == 1)
    {
        signal(SIGUSR1, ProcessTerminated);
        signal(SIGUSR2,SIG_IGN);
        process_count = atoi(argv[3]); // number of processes
        // process_count = 3;
        // struct proc ReadyQ[20] = {0};
        struct PQNode *ReadyQ = NULL;
        Running = NULL;
        int last_runclk;

        while (process_count > 0)
        {

            struct msgbuff message;
            struct Tnode* newprocess;
            int rec_value = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
            while (rec_value != -1)
            {
                //printf("received: %d at time %d \n", message.process.memorysize, getClk());

                utilization += message.process.runtime;
                
                newprocess=allocateprocess(message.process);

                struct PQNode *newnode = PQnewNode(&message.process, message.process.priority, -1, message.process.runtime, 0,newprocess->start,newprocess->end ,arrived);

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
            int flag = 0;
            if (PQisEmpty(&Running) && !PQisEmpty(&ReadyQ))
            {
                Running = PQDeQueue(&ReadyQ);
                flag = 1;
            }
            if (flag == 1 && !PQisEmpty(&Running))
            {
                if (Running->processPID == -1)
                {
                    // fork and take id of process
                    int pid = fork();
                    if (pid == 0)
                    {
                        char *runtime_char = malloc(sizeof(char));
                        sprintf(runtime_char, "%d", Running->process.runtime);
                        char *arg[] = {runtime_char, NULL};
                        int execute = execv("./process.out", arg);
                        if (execute == -1)
                            printf("failed to execute process\n");
                        perror("The error is: \n");
                        exit(-1);
                    }
                    raise(SIGSTOP);
                    Running->processPID = pid;
                    Running->status = started;
                    Running->WaitingTime = (getClk() - Running->process.arrvialtime) - (Running->process.runtime - Running->ReaminingTime);
                    fprintf(logfile, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), Running->process.id, Running->process.arrvialtime, Running->process.runtime, Running->ReaminingTime, Running->WaitingTime);
                }
            }
        }
        wta /= atoi(argv[3]);
        avg_wait /= atoi(argv[3]);
        utilization /= getClk();
        utilization *= 100;
    }
    ///////////////////////////////// HPF
    else if (atoi(argv[1]) == 2)
    {
        signal(SIGUSR2, SIG_IGN);
        signal(SIGUSR1, ProcessTerminated);
        process_count = atoi(argv[3]); // number of processes
        struct PQNode *ReadyQ = NULL;
        Running = NULL; //points to the currently running process node
        int last_runclk; //holds the value of the clock at start/continue for the currently running process

        while (process_count > 0)
        {
            struct msgbuff message;
            struct Tnode* newprocess;
            int rec_value = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
            while (rec_value != -1)
            {

                printf("received: %d at time %d \n", message.process.id, getClk());
                utilization += message.process.runtime;
                message.process.arrvialtime=getClk();
                newprocess=allocateprocess(message.process);

                //enqueuing the arrived process
                struct PQNode *newnode = PQnewNode(&message.process, message.process.priority, -1, message.process.runtime, 0,newprocess->start,newprocess->end, arrived);
                    
                PQEnQueue(&ReadyQ, newnode);
                // printf("head %d\n", ReadyQ->process.id);
                //  if (Running != NULL)
                //  {
                //      printf("running %d\n", Running->process.id);
                //      printf("checking condition %d\n", peek(&ReadyQ).priority < Running->process.priority);
                //  }
                rec_value = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
            }

            //indicates whether a new process has been dequeued to run
            int flag = 0;
            //case that no process is currently running
            if (PQisEmpty(&Running) && !PQisEmpty(&ReadyQ))
            {
                // dequeuing the process to run
                // printf("before dequeuing %d\n", ReadyQ->process.id);
                Running = PQDeQueue(&ReadyQ);
                // printf("after dequeuing running is empty %d %d is empty q %d \n", isEmpty(&Running), Running->process.id, isEmpty(&ReadyQ));
                flag = 1;
            }
            //case that the recieved process has higher priority than the running process (pre-empt)
            else if (!PQisEmpty(&Running) && !PQisEmpty(&ReadyQ) && (PQpeek(&ReadyQ).priority < Running->process.priority))
            {
                // stopping the old process
                // printf("at stop signal running pid=%d head pid=%d \n", Running->processPID, peek(&ReadyQ).id);
                int iskill = kill(Running->processPID, SIGSTOP);
                Running->ReaminingTime += last_runclk - getClk();
                fprintf(logfile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), Running->process.id, Running->process.arrvialtime, Running->process.runtime, Running->ReaminingTime, Running->WaitingTime);
                // printf("isstopped %d %d \n", iskill, Running->processPID);
                Running->status = stopped;
                PQEnQueue(&ReadyQ, Running);
                Running = PQDeQueue(&ReadyQ);
                flag = 1;
            }

            if (flag == 1)
            {
                //if the process is new and hasnt been forked before
                if (Running->processPID == -1)
                {
                    // fork and take id of process
                    int pid = fork();
                    if (pid == 0)
                    {
                        char *runtime_char = malloc(sizeof(char));
                        sprintf(runtime_char, "%d", Running->process.runtime);
                        // printf("runtime %s of process %d time rn %d\n",runtime_char,getpid(),getClk());
                        char *arg[] = {runtime_char, NULL};
                        int execute = execv("./process.out", arg);
                        // error handling
                        if (execute == -1)
                            printf("failed to execute process\n");
                        perror("The error is: \n");
                        exit(-1);
                    }
                    //pausing the scheduler till the process is forked so that the clock is synced between the files (gets resumed from inside the process.c file)
                    raise(SIGSTOP);
                    Running->processPID = pid;
                    Running->status = started;
                    last_runclk = getClk();
                    Running->WaitingTime = (getClk() - Running->process.arrvialtime) - (Running->process.runtime - Running->ReaminingTime);
                    fprintf(logfile, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), Running->process.id, Running->process.arrvialtime, Running->process.runtime, Running->ReaminingTime, Running->WaitingTime);
                }
                //if the process is continuing (has been forked and preempted before)
                else
                {
                    // printf("continuing signal %d\n", Running->processPID);
                    kill(Running->processPID, SIGCONT);
                    Running->status = resumed;
                    last_runclk = getClk();
                    // printf("resumed remaining time %d\n",Running->ReaminingTime);
                    // waiting time = (now-arrival time) - (runtime-remaining time) [total time in system - total time ive run]
                    Running->WaitingTime = (getClk() - Running->process.arrvialtime) - (Running->process.runtime - Running->ReaminingTime);
                    fprintf(logfile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), Running->process.id, Running->process.arrvialtime, Running->process.runtime, Running->ReaminingTime, Running->WaitingTime);
                }
            }
        }
        //.perf file calculations
        wta /= atoi(argv[3]);
        avg_wait /= atoi(argv[3]);
        utilization /= getClk();
        utilization *= 100;
    }
    ///////////////////////////////// RR
    else if (atoi(argv[1]) == 3)
    {
        signal(SIGUSR1, SIG_IGN);
        signal(SIGUSR2, clockchange);
        process_count = atoi(argv[3]);
        struct PQNode *ReadyQ = NULL;
        Running = NULL;
        quantum = atoi(argv[2]);
        int flag = 0;
        int last_runclk = 0;
        int prevClk = getClk();
        while (process_count > 0)
        {
            struct msgbuff message;
            struct Tnode* newprocess;
            int rec = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
            while (rec != -1)
            {
                printf("received: %d at time %d \n", message.process.id, getClk());
                utilization += message.process.runtime;
                newprocess=allocateprocess(message.process);
                struct PQNode *newnode = PQnewNode(&message.process, 10, -1, message.process.runtime, 0, newprocess->start,newprocess->end, arrived);
                // printf("Run time is %d\n", newnode->ReaminingTime);
                PQEnQueue(&ReadyQ, newnode);
                rec = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
                // process_count--;
                // printf("%d\n",process_count);
            }
            if (Running == NULL && ReadyQ != NULL)
            {
                Running = PQDeQueue(&ReadyQ);
                count = 0; //counts the number of seconds that pass as a process is running
                //printf("running process %d\n", Running->process.id);
                quantum = atoi(argv[2]);
                //in case that the remaining time is less than the quantum then process runs to completion
                if (Running->ReaminingTime < quantum)
                {
                    quantum = Running->ReaminingTime;
                }

                //if the process hasnt been forked before
                if (Running->processPID == -1)
                {
                    //printf("forking\n");
                    int pid = fork();
                    if (pid == 0)
                    {
                        char *runtime_char = malloc(sizeof(char));
                        sprintf(runtime_char, "%d", Running->process.runtime);
                        // printf("runtime %s of process %d time rn %d\n", runtime_char, getpid(), getClk());
                        char *arg[] = {runtime_char, NULL};
                        int execute = execv("./process.out", arg);
                        if (execute == -1)
                            printf("failed to execute process\n");
                        perror("The error is: \n");
                        exit(-1);
                    }
                    Running->processPID = pid;
                    Running->status = started;
                    Running->WaitingTime = (getClk() - Running->process.arrvialtime) - (Running->process.runtime - Running->ReaminingTime);
                    raise(SIGSTOP);
                    fprintf(logfile, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), Running->process.id, Running->process.arrvialtime, Running->process.runtime, Running->ReaminingTime, Running->WaitingTime);
                }
                //if the process has been pre-empted before
                else
                {
                   // printf("continuing process\n");
                    kill(Running->processPID, SIGCONT);
                    Running->status = resumed;
                    Running->WaitingTime = (getClk() - Running->process.arrvialtime) - (Running->process.runtime - Running->ReaminingTime);
                    fprintf(logfile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), Running->process.id, Running->process.arrvialtime, Running->process.runtime, Running->ReaminingTime, Running->WaitingTime);
                }
            }
            //stopping the running process when it finishes its quantum
            if (count == quantum)
            {
                //printf("reached end of quantum, count %d at time %d\n", count, getClk());
                //printf("rem time in sched %d\n", Running->ReaminingTime);

                //process finished
                if (Running->ReaminingTime == 0)
                {
                    int status;
                    //waiting for the process to terminate
                    int rec_child = waitpid(Running->processPID, &status, 0);
                    //printf("rec child %d\n", rec_child);
                    Running->status = finished;
                    process_count--;
                    avg_wait += Running->WaitingTime;
                    wta += (float)(getClk() - Running->process.arrvialtime) / Running->process.runtime;
                    Running->WaitingTime = (getClk() - Running->process.arrvialtime) - (Running->process.runtime - Running->ReaminingTime);
                    fprintf(logfile, "At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", getClk(), Running->process.id, Running->process.arrvialtime, Running->process.runtime, Running->WaitingTime, getClk() - Running->process.arrvialtime, (float)(getClk() - Running->process.arrvialtime) / Running->process.runtime);
                    deallocateprocess(Running->process,Running->startaddress,Running->endaddress);
                    Running = NULL;
                    count=0;
                }
                //pausing the currently running process
                else
                {
                    //case that the running process is not the only process in the queue
                    if (ReadyQ != NULL)
                    {
                        //pause the running process to run another
                        kill(Running->processPID, SIGSTOP);
                        Running->status = stopped;
                        Running->WaitingTime = (getClk() - Running->process.arrvialtime) - (Running->process.runtime - Running->ReaminingTime);
                        fprintf(logfile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), Running->process.id, Running->process.arrvialtime, Running->process.runtime, Running->ReaminingTime, Running->WaitingTime);
                        // enqueuing arrived processes
                        int rec = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
                        while (rec != -1)
                        {
                            //printf("received: %d at time %d \n", message.process.id, getClk());
                            utilization += message.process.runtime;
                            newprocess=allocateprocess(message.process);
                            struct PQNode *newnode = PQnewNode(&message.process, 10, -1, message.process.runtime, 0,newprocess->start,newprocess->end, arrived);
                            PQEnQueue(&ReadyQ, newnode);
                            rec = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
                        }
                        PQEnQueue(&ReadyQ, Running);
                        Running = NULL;
                    }
                    else
                    {
                        //giving the currently running process another quantum since its the only process in the queue (instead of pausing and continuing the same process)
                        count = 0;
                        quantum = atoi(argv[2]);
                        if (Running->ReaminingTime < quantum)
                        {
                            quantum = Running->ReaminingTime;
                        }
                    }
                }
            }
        }
        //.perf file calculations
        wta /= atoi(argv[3]);
        avg_wait /= atoi(argv[3]);
        utilization /= getClk();
        utilization *= 100;
    }
    ///////////////////////////////// MLFL
    else if (atoi(argv[1]) == 4) 
    {
        signal(SIGUSR1, ProcessTerminated);
        signal(SIGUSR2,SIG_IGN);
        char clk;
        int hold_key; //hold key is the key at which we know that we have already completed one loop.
        int lowest_priority; //To indicate the lowest priority in our loop (a large number).
        int isEmp; //indicates whether the queue is empty in the pervoius dequeue-ing.
        int stop =0;  //Force "hold_key" to store the key of the FIRST least-priorty-executed process not the LAST.
        int counter=1; //Counts the number of priorties stand still in the queue after every loop.
        int flags[13]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}; //Indicate whether we are filling the queue or the linked list extended from the queue.
        int flags_sync[13]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
        int Degrader=1; //indiacte whether the process gonna be degraded by one or more level. 
        int Resnap_indicator=0; //Set the flags to zero, to start the re-snaping.
        int incrementer=0;  
        int stopped_at=0;
        int fixed_increm=0;
        int least_priority_now=-100;
        int a=0;
        node* current=NULL; //Works as temporary pointer to move around in Live_latch extended chain.
        node *Live_latch[11]; //Hold the updated snap of the MLFL after every loop.
        node *latch[11];  //Holds the initial snap of the MLFL. 
        struct QNode* Q0=NULL; //Defining the main queue.
        int how_much_now=0;
        //Filling both array with NULLs, as an initial state.
        for (int i=0; i<11; i++){
        latch[i] = malloc(sizeof(node));
        Live_latch[i] = malloc(sizeof(node));
        Live_latch[i]=NULL;
        latch[i]=NULL;
        }

        quantum = atoi(argv[2]);
        process_count = atoi(argv[3]);
        int i=0;
        node processes[2];
        struct msgbuff message;
        int rec; 
   

        while (process_count >0){

            int stope=0;
            a=0;
            rec = msgrcv(msqid, &message, sizeof(message.process), 0, !IPC_NOWAIT);
            least_priority_now=message.process.priority;

        while (!isEmpty(&Q0) || a==0){   
            a=1;

            if (stope!=0)
            rec = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
            stope=1;

            while (rec != -1)
                {
                    printf("received: %d at time %d \n", message.process.id, getClk()); fflush(stdout);
                    utilization += message.process.runtime;
                    processes[incrementer].key=message.process.id;
                    processes[incrementer].arrival_time=message.process.arrvialtime;
                    processes[incrementer].priority=message.process.priority;
                    processes[incrementer].runtime=message.process.runtime;
                    processes[incrementer].remaining_Time=message.process.runtime;
                    incrementer++;
                    rec = msgrcv(msqid, &message, sizeof(message.process), 0, IPC_NOWAIT);
                    how_much_now++;
                    if (message.process.priority>least_priority_now)
                    least_priority_now=message.process.priority;
                }

                   

                    //Looping on the given processes, to fill the queue of "nodes",
                    //i.e convert from the given struct to node struct

                    for (int i=fixed_increm; i<how_much_now; i++){
                    struct node* process= (struct node*)malloc(sizeof(struct node));
                    process->key=processes[i].key;
                    process->priority=processes[i].priority;
                    process->Live_Priority=processes[i].priority;
                    process->arrival_time=processes[i].arrival_time;
                    process->runtime=processes[i].runtime;
                    process->remaining_Time=processes[i].remaining_Time;
                    process->pid=-1;
                    fixed_increm++;           

          
                    if (flags_sync[process->priority] ==-1){ //If it is the first process in particular priority, Enqueue it.

                        latch[process->priority]=process;
                        flags_sync[process->priority]++;
                    
                        if (isEmpty(&Q0)){
                        Q0 = newNode(process);
                        }
                        else{

                        EnQueue(&Q0,process);
                        }

                    }
                    
                    else {  //If it is NOT the first process in particular priority, link it to the chain.
                        insertAfter(latch[process->priority],process);
                        latch[process->priority]=process;
                    }

                    }

                    PrintQueue(&Q0); //Print to show the initial state of the MLFL.   


                    //Here the algoritm starts
                
                    node* temp=DeQueue(&Q0); //This is the process meant to be executed

                    if (temp == NULL){}
                    else{
                
                   int Clock_now=getClk();
                   int pid;
                   int passed_here_flag=0;
                    if (temp!=NULL && temp->pid==-1){
                        passed_here_flag=1;
                        pid=fork();
                        if (pid ==0){ //Only Child Enters
                            char *remaining_Time_char = malloc(sizeof(char));
                            char *quantum_char = malloc(sizeof(char));
                            sprintf(remaining_Time_char, "%d", temp->remaining_Time);
                            sprintf(quantum_char, "%d", quantum);

                            char *arg[] = {remaining_Time_char,quantum_char, NULL};
                            int execute = execv("./process.out", arg);
                            // error handling
                            if (execute == -1)
                                printf("failed to execute process\n");
                            perror("The error is: \n");
                            exit(-1);
                        }
                        temp->wating_Time = (getClk() - temp->arrival_time) - (temp->runtime- temp->remaining_Time);
                        fprintf(logfile, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), temp->key, temp->arrival_time, temp->runtime, temp->remaining_Time, temp->wating_Time);
 
                        temp->pid=pid;
                        //system("ps");

                        while (getClk()!=Clock_now+quantum){
                        }
                        
                        kill(temp->pid, SIGSTOP);
                        temp->wating_Time = (getClk() - temp->arrival_time) - (temp->runtime- temp->remaining_Time);
                        fprintf(logfile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), temp->key, temp->arrival_time, temp->runtime, temp->remaining_Time, temp->wating_Time-1);
                              
                    }
                    
                    else if (temp!=NULL && temp->pid!=-1 && pid!=0 && passed_here_flag==0){
                         Clock_now=getClk(); 
                         kill(temp->pid, SIGCONT);
                         

                        temp->wating_Time = (getClk() - temp->arrival_time) - (temp->runtime- temp->remaining_Time);
                        fprintf(logfile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), temp->key, temp->arrival_time, temp->runtime, temp->remaining_Time, temp->wating_Time);

                           
                    while (getClk()!=Clock_now+quantum){
                        }
                        kill(temp->pid, SIGSTOP);
                        temp->wating_Time = (getClk() - temp->arrival_time) - (temp->runtime- temp->remaining_Time);
                        fprintf(logfile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), temp->key, temp->arrival_time, temp->runtime, temp->remaining_Time, temp->wating_Time-1);

                    }
                  
                

                    if (isEmpty(&Q0)){
                    isEmp =1;
                    }
                    else 
                    isEmp=0;
                
                    if (temp->remaining_Time>0){
                        if (temp->remaining_Time<=quantum)
                            quantum=temp->remaining_Time;
                    temp->remaining_Time=temp->remaining_Time-quantum;        //Deduct one quantum from process's runtime 
                    }  
                    quantum = atoi(argv[2]);                                                                   

                    //Enqueue-ing the following process in the chain into the queue.
                    if (temp->next!=NULL){  
                        if (isEmpty(&Q0)){
                            Q0=newNode(temp->next);
                        }
                        else{
                            EnQueue(&Q0,temp->next);
                        }
                    }
                    
                    //Now we have two main cases, whether the process has already finished or still has remaining runtime.
                    //Case1: Process has already finished
                    if (temp->remaining_Time==0){
                         process_count--;
                    
                    //int TA= getClk()-temp->arrival_time;
                    //int WTA = (getClk()-temp->arrival_time)/process_count;
                     temp->wating_Time = (getClk() - temp->arrival_time) - (temp->runtime- temp->remaining_Time);
                     avg_wait += (temp->wating_Time);
                     wta+=(float) ((getClk() - temp->arrival_time) / temp->runtime);
                    fprintf(logfile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %f\n", getClk(), temp->key, temp->arrival_time, temp->runtime, temp->remaining_Time, temp->wating_Time,getClk()-temp->arrival_time,(float)(getClk() - temp->arrival_time) / temp->runtime);
                
                        flags_sync[temp->priority]--;

                    }

                    //Case2: Process still has remaining runtime.
                    else {
                    /*Now we have two sub cases: whether we are degrading each process to lower level, or we are finishing 
                    a specific snap to start a new one*/

                    //Subcase1: Degrading each process to lower level
                        if ((temp->Live_Priority!=10 || temp->Live_Priority==temp->priority) && isEmp==0 ) {

                            if (temp->Live_Priority==11 ){
                                Degrader=0;
                            }
                            //Handling: Conrner Case of degrading more than one level. 
                            while(latch[temp->Live_Priority+Degrader]==NULL){
                                Degrader++;
                            
                            } 
                            latch[temp->Live_Priority+Degrader]->next=temp;
                              
                            latch[temp->Live_Priority+Degrader]=temp;
                            temp->Live_Priority=temp->Live_Priority+Degrader;
                            temp->next=NULL;
                            Degrader=1;

                            //Storing the hold key
                            if (temp->Live_Priority==10 && temp->priority==10 && stop==0){
                                hold_key=temp->key;
                                stop++;
                            }

                        
                        }

                    //Subcase2: finishing a specific snap to start a new one
                        else if ((temp->Live_Priority== 10 && temp->Live_Priority!=temp->priority) || isEmp==1 ){
                            temp->Live_Priority=temp->priority;  //Returning priority to its origin.

                            if (Resnap_indicator==0){

                            
                            flags[0]=-1; flags[1]=-1; flags[2]=-1; flags[3]=-1; flags[4]=-1; flags[5]=-1;
                            flags[6]=-1; flags[7]=-1; flags[8]=-1; flags[9]=-1; flags[10]=-1; flags[11]=-1;
                            flags[12]=-1;  
                            Resnap_indicator++;
                            }
                          
                            if (flags[temp->Live_Priority]==-1 && temp->remaining_Time!=0 ){ //It is the first process in particular priority

                                //if(current!=NULL && current->priority!=temp->priority){ //Cleaning the chain
                                //current->next=NULL;
                                //}
     
                                Live_latch[temp->Live_Priority]=temp;
                                flags[temp->Live_Priority]=temp->priority;
                                counter++;
                                current = Live_latch[temp->Live_Priority];
                                  
                            }

                            else { //It is NOT the first process in particular priority

                                if(current->priority==temp->priority){ 
                                current->next=temp;
                                current=current->next;
                                }

                                else {
                                current->next=NULL;
                                }
                            }
                        }
                        
                    }

                    //===============================================Re-Snaping============================================// 
                    if ((!isEmpty(&Q0) && temp->next!=NULL && temp->next->key==hold_key) || (isEmpty(&Q0) && temp->next==NULL) ){

                        for (int i=0; i<=10; i++){      
                            if(Live_latch[i]!=NULL){
                            cleanChain(Live_latch[i]);

                                if (!isEmpty(&Q0)){
                                EnQueue(&Q0,Live_latch[i]);
                                }
                                else if (isEmpty(&Q0)){
                                Q0=newNode(Live_latch[i]);

                                }
                            
                                latch[i]=LastNodeInChain(Live_latch[i]);
                                Live_latch[i]=NULL;
                                 flags[i]=-1;
                                 //flags_sync[i]=Live_latch[i]->priority;

                                }
                                
                                else{
                                    latch[i]=NULL;

                                }

                            }
                            //Returing all indicator variables to its initial state.
                            counter=0;       
                            Degrader=1;
                            stop=0;
                            hold_key=0;
                    }
                    //============================================End Re-Snaping===============================================// 
                    //if (!isEmpty(&Q0)){
                    //PrintQueue(&Q0);
                    //}

                }
                  }
                    }
                        }

    printf("exiting scheduler at time %d\n", getClk());

    // printing to .perf file
    //utilization= (utilization/getClk())*100;
    wta /= atoi(argv[3]);
    avg_wait /= atoi(argv[3]);
    fprintf(perf, "CPU utilization= %.2f %% Avg \nWTA =%.2f \nAvg Waiting =%.2f\n", utilization, wta, avg_wait);

    fclose(perf);
    fclose(logfile);
    fclose(memoryfile);

    // releasing communication with the clock module
    destroyClk(1);

    return 0;
}

void ProcessTerminated(int signum)
{
    //.perf file calculations
    avg_wait += Running->WaitingTime;
    wta += (float)(getClk() - Running->process.arrvialtime) / Running->process.runtime;
    fprintf(logfile, "At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", getClk(), Running->process.id, Running->process.arrvialtime, Running->process.runtime, Running->WaitingTime, getClk() - Running->process.arrvialtime, (float)(getClk() - Running->process.arrvialtime) / Running->process.runtime);
    //printf("At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", getClk(), Running->process.id, Running->process.arrvialtime, Running->process.runtime, Running->WaitingTime, getClk() - Running->process.arrvialtime, (float)(getClk() - Running->process.arrvialtime) / Running->process.runtime);
    deallocateprocess(Running->process,Running->startaddress,Running->endaddress);
    free(Running);
    Running = NULL;
    process_count--;
    //printf("process_count=%d\n", process_count);
}

void clockchange(int signum)
{
    count++;
    Running->ReaminingTime--;
    //printf("count: %d rem time %d\n", count, Running->ReaminingTime);
}
