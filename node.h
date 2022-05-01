#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#ifndef MYNODE_H_
#define MYNODE_H_
typedef struct node {
   int key;          //ID
   int arrival_time;
   int priority;
   int runtime;
   int data;
   int Live_Priority;
   struct node *next;
} node;
#endif
