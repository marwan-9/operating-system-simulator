#include <stdio.h>
#include "LinkedList.c"
#include "queue.c"

int main (){
   struct process {
   int ID;
   int arrival_time;
   int priority;
   int runtime;
};
int p =10;
int flags[]={0,0,0,0,0,0,0,0,0,0};
node *latch[50];
for (int i=0; i<50; i++){
   latch[i] = malloc(sizeof(node));
}


 node processes[12];

processes[0].key = 1; processes[0].priority=4; processes[0].arrival_time = 2;  processes[0].runtime = 4; 
processes[1].key = 2; processes[1].priority=2; processes[1].arrival_time = 3; processes[1].runtime = 7; 
processes[2].key = 3; processes[2].priority=1; processes[2].arrival_time = 5;  processes[2].runtime = 2; 
processes[3].key = 4; processes[3].priority=5; processes[3].arrival_time = 6;  processes[3].runtime = 3; 
processes[4].key = 5; processes[4].priority=2; processes[4].arrival_time = 8; processes[4].runtime = 5; 
processes[5].key = 6; processes[5].priority=3; processes[5].arrival_time = 10;  processes[5].runtime = 6; 
processes[6].key = 7; processes[6].priority=5; processes[6].arrival_time = 12;  processes[6].runtime = 1; 
processes[7].key = 8; processes[7].priority=5; processes[7].arrival_time = 15;  processes[7].runtime = 3;

processes[8].key = 9;   processes[8].priority=2; processes[8].arrival_time = 8; processes[8].runtime = 5; 
processes[9].key = 10;  processes[9].priority=4; processes[9].arrival_time = 10;  processes[9].runtime = 6; 
processes[10].key = 11; processes[10].priority=1; processes[10].arrival_time = 12;  processes[10].runtime = 1; 
processes[11].key = 12; processes[11].priority=5; processes[11].arrival_time = 15;  processes[11].runtime = 3;
//Processes alreadry defined for testing (sizeof processes / sizeof processes[0])
struct QNode* Q0;

for (int i=0; i<(sizeof(processes)/ sizeof(processes[0])); i++){
struct node* process= (struct node*)malloc(sizeof(struct node));
process->key=processes[i].key;
process->priority=processes[i].priority;
process->arrival_time=processes[i].arrival_time;

 if (flags[process->priority] ==0){
    latch[process->priority]=process;
    flags[process->priority]=process->priority;
    if (i==0){
    Q0 = newNode(process);
    }
    else{
      
       EnQueue(&Q0,process);
    }

 }
 else {
   insertAfter(latch[process->priority],process);
    latch[process->priority]=process;
 }
}
   PrintQueue(&Q0);

//======================================PRINT QUEUE CODE ENDED===================================//


char clk;
int aux=0;
scanf(" %c", &clk);
  while (clk=='c' && !isEmpty(&Q0)){
   scanf(" %c", &clk);
  }



 }
