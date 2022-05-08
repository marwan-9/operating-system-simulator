//C code to implement Priority Queue
// using Linked List
#include "headers.h"
 
// Node
struct PQNode {
    
    int Qpriority;
    struct process process;
    struct PQNode* next;
    int processPID;
    int ReaminingTime;
    int WaitingTime;
    enum status status;
};


// Function to Create A New Node
struct PQNode* PQnewNode(struct process* input, int Qpriority,int Pid,int remainingtime, int waitingtime, enum status status)
{
    struct PQNode* temp = (struct PQNode*)malloc(sizeof(struct PQNode));
    temp->process = *input;
    temp->Qpriority = Qpriority;
    temp->processPID=Pid;
    temp->ReaminingTime=remainingtime;
    temp->WaitingTime=waitingtime;
    temp->status=status;
    temp->next = NULL;
    return temp;
}
 
// Return the value at head
struct process PQpeek(struct PQNode** head)
{
    return (*head)->process;
}
 
// Removes the element with the
// highest priority form the list
struct PQNode* PQDeQueue(struct PQNode** head)
{
    struct PQNode *Dequeued; 
    //struct PQNode* temp = *head;
	Dequeued = ((*head));
     //printf("here?"); fflush(stdout);
    (*head) = (*head)->next;
    
    //free(temp);
    return Dequeued;
}
 
// Function to push according to priority
void PQEnQueue(struct PQNode** head,struct PQNode* newnode)
{
    struct PQNode* start = (*head);
    // Create new Node
    //struct PQNode* temp = newNode(input,Qpriority,Pid,remainingtime,waitingtime,status);
    if(*head==NULL){
        *head=newnode;
        return;
    }
    // Special Case: The head of list has lesser
    // priority than new node. So insert new
    // node before head node and change head node.
	int p = newnode->Qpriority;
    if ((*head)->Qpriority > p) {
        // Insert New Node before head
        newnode->next = *head;
        (*head) = newnode;
    }   

    else {
        // Traverse the list and find a
        // position to insert new node
        if (start !=NULL){
        while (start->next!=NULL && (start->next)->Qpriority < p) {
            start = start->next;
        }
    }
        // Either at the ends of the list
        // or at required position
        newnode->next = start->next;
        start->next = newnode;

    }
}
 //Added//////////////////
//Insert ascendingly according to the shorter runtime or burst
void Enqueue_RT(struct PQNode** head,struct PQNode* newnode)
{
    struct PQNode* start = (*head);
    
    if(*head==NULL){
        *head=newnode;
        return;
    }
    // Special Case: The head of list has lesser
    // runtime than new node. So insert new
    // node before head node and change head node.
	int p = newnode->ReaminingTime;
    if ((*head)->ReaminingTime > p) {
        // Insert New Node before head
        newnode->next = *head;
        (*head) = newnode;
    }   

    else {
        // Traverse the list and find a
        // position to insert new node
        if (start !=NULL){
        while (start->next!=NULL && (start->next)->ReaminingTime < p) {
            start = start->next;
        }
    }
        // Either at the ends of the list
        // or at required position
        newnode->next = start->next;
        start->next = newnode;

    }
    
}
// Function to check is list is empty
int PQisEmpty(struct PQNode** head)
{
    return (*head) == NULL;
}

