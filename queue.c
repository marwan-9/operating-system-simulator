//C code to implement Priority Queue
// using Linked List
#include <stdio.h>
#include <stdlib.h>
#include "node.h"
 
// Node
struct QNode {
	int data;
    struct node process; 
  // int priority;
    struct QNode* next;
};


// Function to Create A New Node
struct QNode* newNode(struct node* input)
{
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->process = *input;
    temp->process.priority = input->priority;
    temp->next = NULL;
    return temp;
}
 
// Return the value at head
struct node peek(struct QNode** head)
{
    return (*head)->process;
}
 
// Removes the element with the
// highest priority form the list
node* DeQueue(struct QNode** head)
{
    node *Dequeued; 
    struct QNode* temp = *head;
	Dequeued = &((*head)->process);
    //printf("yarab %d \n",Dequeued->priority);
    (*head) = (*head)->next;

    free(temp);
    return Dequeued;
}
 
// Function to push according to priority
void EnQueue(struct QNode** head,struct node input)
{
    struct QNode* start = (*head);
    // Create new Node
    struct QNode* temp = newNode(&input);
    // Special Case: The head of list has lesser
    // priority than new node. So insert new
    // node before head node and change head node.
	int p = input.priority;
    if ((*head)->process.priority > p) {
        // Insert New Node before head
        temp->next = *head;
        (*head) = temp;
    }   

    else {
        // Traverse the list and find a
        // position to insert new node
        if (start !=NULL){
        while (start->next!=NULL && (start->next)->process.priority < p) {
            start = start->next;
        }
    }
        // Either at the ends of the list
        // or at required position
        temp->next = start->next;
        start->next = temp;

    }
}
 
// Function to check is list is empty
int isEmpty(struct QNode** head)
{
    return (*head) == NULL;
}