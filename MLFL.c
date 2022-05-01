//This is a real simulation of MLFL algorithm using a "queue of linked list"
//Snap: is the static state of the MLFL after every loop, and we mean by loop that all levels have taken a quantum
#include <stdio.h>
#include "LinkedList.c"
#include "queue.c"
int main (){

//==================================TESTING PART (GONNA BE DELETED LATER)==================================//
//Assumed Process Structure   
struct process {
int ID;
int arrival_time;
int priority;
int runtime;
};

char clk;
node processes[4];
processes[0].key = 1; processes[0].priority=1; processes[0].arrival_time = 2;  processes[0].runtime = 1; 
processes[1].key = 2; processes[1].priority=2; processes[1].arrival_time = 3; processes[1].runtime = 50; 
processes[2].key = 3; processes[2].priority=3; processes[2].arrival_time = 5;  processes[2].runtime = 1; 
processes[3].key = 4; processes[3].priority=4; processes[3].arrival_time = 6;  processes[3].runtime = 5; 
processes[4].key = 5; processes[4].priority=2; processes[4].arrival_time = 8; processes[4].runtime = 1; 
processes[5].key = 6; processes[5].priority=3; processes[5].arrival_time = 10;  processes[5].runtime = 6; 
processes[6].key = 7; processes[6].priority=4; processes[6].arrival_time = 12;  processes[6].runtime = 7; 
processes[7].key = 8; processes[7].priority=3; processes[7].arrival_time = 15;  processes[7].runtime = 1;
processes[8].key = 9;   processes[8].priority=5; processes[8].arrival_time = 8; processes[8].runtime = 7; 
processes[9].key = 10;  processes[9].priority=5; processes[9].arrival_time = 10;  processes[9].runtime = 6; 
processes[10].key = 11; processes[10].priority=4; processes[10].arrival_time = 12;  processes[10].runtime = 5; 
processes[11].key = 12; processes[11].priority=5; processes[11].arrival_time = 15;  processes[11].runtime = 10;
//============================================TESTING PART ENDED==========================================//

//Variables Defining:
//Integers:
int hold_key; //hold key is the key at which we know that we have already completed one loop.
int lowest_priority; //To indicate the lowest priority in our loop (a large number).
int isEmp; //indicates whether the queue is empty in the pervoius dequeue-ing.
int stop =0;  //Force "hold_key" to store the key of the FIRST least-priorty-executed process not the LAST.
int counter=1; //Counts the number of priorties stand still in the queue after every loop.
int flags[50]={0}; //Indicate whether we are filling the queue or the linked list extended from the queue.
int Degrader=1; //indiacte whether the process gonna be degraded by one or more level. 
int Resnap_indicator=0; //Set the flags to zero, to start the re-snaping.

node* current=NULL; //Works as temporary pointer to move around in Live_latch extended chain.
node *Live_latch[50]; //Hold the updated snap of the MLFL after every loop.
node *latch[50];  //Holds the initial snap of the MLFL. 
struct QNode* Q0; //Defining the main queue.

//Filling both array with NULLs, as an initial state.
for (int i=0; i<50; i++){
   latch[i] = malloc(sizeof(node));
   Live_latch[i] = malloc(sizeof(node));
   Live_latch[i]=NULL;
}

//Looping on the given processes, to fill the queue of "nodes", i.e convert from the given struct to node struct
for (int i=0; i<(sizeof(processes)/ sizeof(processes[0])); i++){
struct node* process= (struct node*)malloc(sizeof(struct node));
process->key=processes[i].key;
process->priority=processes[i].priority;
process->Live_Priority=processes[i].priority;
process->arrival_time=processes[i].arrival_time;
process->runtime=processes[i].runtime;

 if (flags[process->priority] ==0){ //If it is the first process in particular priority, Enqueue it.
    latch[process->priority]=process;
    flags[process->priority]=process->priority;
    if (i==0){
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


scanf(" %c", &clk); //Fake clock (TO BE DELETED)
//Here the algoritm starts
  while (clk=='c' && !isEmpty(&Q0)){
   node* temp=DeQueue(&Q0); //This is the process meant to be executed

   if (isEmpty(&Q0))
   isEmp =1;
   else 
   isEmp=0;
   
   temp->runtime=temp->runtime-1; //Deduct one quantum from process's runtime 

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
  if (temp->runtime==0){
   }

  //Case2: Process still has remaining runtime.
  else {

  /*Now we have two sub cases: whether we are degrading each process to lower level, or we are finishing 
  a specific snap to start a new one*/

   //Subcase1: Degrading each process to lower level
     if ((temp->Live_Priority!=5 || temp->Live_Priority==temp->priority) && isEmp==0 ) {
      
         if (temp->Live_Priority==5){
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
         if (temp->Live_Priority==5 && temp->priority==5 && stop==0){
            hold_key=temp->key;
            stop++;
         }
     
     }

   //Subcase2: finishing a specific snap to start a new one
     else if (temp->Live_Priority==5 && temp->Live_Priority!=temp->priority || isEmp==1 ){
       
        temp->Live_Priority=temp->priority;  //Returning priority to its origin.

        if (Resnap_indicator==0){
        memset(flags,0, 50);
        Resnap_indicator++;
        }

        if (flags[temp->Live_Priority]==0 && temp->runtime!=0 ){ //It is the first process in particular priority

            if(current!=NULL && current->priority!=temp->priority){ //Cleaning the chain
               current->next=NULL;
            }
                    
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
      for (int i=1; i<=5; i++){      
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
            flags[i]=0;
            }

            else {
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
  if (!isEmpty(&Q0)){
  PrintQueue(&Q0);
  }

  scanf(" %c", &clk);
 }
}