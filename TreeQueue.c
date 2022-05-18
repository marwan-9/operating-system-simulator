#include <stdio.h>
#include <stdlib.h>
#include "headers.h"

struct node {
	struct Tnode* TreeNode;
	struct node *next;
};

struct node *front = NULL;
struct node *rear = NULL;

// Enqueue() operation on a queue
void enqueue(struct Tnode* input) {
	struct node *ptr;
	ptr = (struct node *)malloc(sizeof(struct node));
	ptr->TreeNode = input;
	ptr->next = NULL;
	if ((front == NULL) && (rear == NULL)) {
		front = rear = ptr;
	} else {
		rear->next = ptr;
		rear = ptr;
	}
	//printf("Node is Inserted\n\n");
}

// Dequeue() operation on a queue
struct Tnode* dequeue() {
	if (front == NULL) {
		//printf("\nUnderflow\n");
		return NULL;
        }
	 else {
		struct node *temp = front;
		struct Tnode* temp_Treenode = front->TreeNode;
		front = front->next;
		free(temp);
		return temp_Treenode;
	}
}

void  Emptyit(){
    for(int i=0; i<128; i++){
        dequeue();
        rear = NULL;
    }
    

}
