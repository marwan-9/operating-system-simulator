#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "node.h"

//FILE *MLFL_Sim;



struct node *head = NULL;
struct node *current = NULL;


//display the list
void printList(node *input) {
   struct node *ptr = input;
   
   //MLFL_Sim = fopen("./MLFL_Sim.txt", "w");
	
   //start from the beginning
   while (ptr!=NULL){
     printf("(#%d,P%d,T%d,LV%d) -->",ptr->key,ptr->priority,ptr->remaining_Time,ptr->Live_Priority);
      //fprintf(MLFL_Sim,"(#%d,P%d,T%d,LV%d) -->",ptr->key,ptr->priority,ptr->remaining_Time,ptr->Live_Priority);
      //fprintf(MLFL_Sim,"\n");
      ptr = ptr->next;
     
   }
   printf("\n");
       //fclose(MLFL_Sim);

	
  // printf(" ]");
}

//insert link at the first location
void insertFirst(int key, int data) {
   //create a link
   struct node *link = (struct node*) malloc(sizeof(struct node));
	
   link->key = key;
   link->data = data;
	
   //point it to old first node
   link->next = head;
	
   //point first to new first node
   head = link;
}

void insertAfter(struct node* prev_node, struct node* new_data)
{

          
    /*1. check if the given prev_node is NULL */
    if (prev_node == NULL) {
        printf("the given previous node cannot be NULL \n");
        return;
    }
  
    /* 2. allocate new node */
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
 
    /* 3. put in the data */
    new_node = new_data;
    /* 4. Make next of new node as next of prev_node */
    new_node->next = prev_node->next;
 
    /* 5. move the next of prev_node as new_node */
    prev_node->next = new_node;



}

void cleanChain(node* Chain_Head){

   int p_ref;

   if (Chain_Head!=NULL){

      p_ref = Chain_Head->priority;

   }


   node* temp = Chain_Head;
                  
   while (temp!=NULL && temp->next!=NULL){

      if (temp->next->priority!=p_ref){
          
         temp->next=NULL;

      }
      
      if (temp->next!=NULL && temp->next->runtime==0){
             
         temp->next=temp->next->next;
      }

      if (temp!=NULL){
      temp=temp->next;
      }

   }

}
node* LastNodeInChain(node* temp){

   while (temp !=NULL && temp->next !=NULL){
      temp=temp->next;
   }
   return temp;
}
//delete first item
struct node* deleteFirst() {

   //save reference to first link
   struct node *tempLink = head;
	
   //mark next to first link as first 
   head = head->next;
	
   //return the deleted link
   return tempLink;
}

//is list empty
bool isEmptyList() {
   return head == NULL;
}

int length() {
   int length = 0;
   struct node *current;
	
   for(current = head; current != NULL; current = current->next) {
      length++;
   }
	
   return length;
}

//find a link with given key
struct node* find(int key) {

   //start from the first link
   struct node* current = head;

   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->key != key) {
	
      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //go to next link
         current = current->next;
      }
   }      
	
   //if data found, return the current Link
   return current;
}

//delete a link with given key
struct node* delete(int key) {

   //start from the first link
   struct node* current = head;
   struct node* previous = NULL;
	
   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->key != key) {

      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //store reference to current link
         previous = current;
         //move to next link
         current = current->next;
      }
   }

   //found a match, update the link
   if(current == head) {
      //change first to point to next link
      head = head->next;
   } else {
      //bypass the current link
      previous->next = current->next;
   }    
	
   return current;
}