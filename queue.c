//C code to implement Priority Queue
// using Linked List
#include <stdio.h>
#include <stdlib.h>
#include "node.h"
 
// Node
struct QNode {
	int data;
    struct node *process; 
    struct QNode* Qnext;
};

int isEmpty(struct QNode** head);
// Function to Create A New Node
struct QNode* newNode(struct node* input)
{
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->process = input;
    //temp->process->priority = input->priority;
    //temp->process->key=input->key;
    temp->Qnext = NULL;

    return temp;
}
 
// Return the value at head
struct node* peek(struct QNode** head)
{
    return (*head)->process;
}
 
// Removes the element with the
// highest priority form the list
node* DeQueue(struct QNode** head)
{
    node *Dequeued; 
    struct QNode* temp = *head;
	Dequeued = ((*head)->process);
     //printf("here?"); fflush(stdout);
    (*head) = (*head)->Qnext;
    
    free(temp);
    return Dequeued;
}
 
// Function to push according to priority
void EnQueue(struct QNode** head,struct node* input)
{
  

  
    struct QNode* start = (*head);
    // Create new Node
    struct QNode* temp = newNode(input);
    // Special Case: The head of list has lesser
    // priority than new node. So insert new
    // node before head node and change head node.
	int p = input->priority;
    if ((*head)->process->priority > p) {
        // Insert New Node before head
        temp->Qnext = *head;
        (*head) = temp;
    }   

    else {
        // Traverse the list and find a
        // position to insert new node
        if (start !=NULL){
        while (start->Qnext!=NULL && (start->Qnext)->process->priority < p) {
            start = start->Qnext;
        }
    }
        // Either at the ends of the list
        // or at required position
        temp->Qnext = start->Qnext;
        start->Qnext = temp;

    }

}
 
// Function to check is list is empty
int isEmpty(struct QNode** head)
{
    return (*head) == NULL;
}


void PrintQueue(struct QNode** Q0){

struct QNode* Q1;
int aux=0;
while (!isEmpty(Q0)){
   node* temp = DeQueue(Q0);
   printList(temp);
   if (aux==0)
   Q1 = newNode(temp);
   else{
   EnQueue(&Q1,temp);
   aux++;
   }
}
   aux=0;
   while (!isEmpty(&Q1)){
      node* temp = DeQueue(&Q1);
      if (aux==0)
      *Q0 = newNode(temp);
      else {
      EnQueue(Q0,temp);
      aux++;
      }
   }
}