#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#ifndef MYNODE_H_
#define MYNODE_H_
typedef struct node {
   int key;          //ID
   int arrival_time;
   signed int priority;
   int runtime;
   int data;
   int Live_Priority;
   int pid;
   struct node *next;
   int wating_Time;
   int remaining_Time;
   int startaddress;
   int endaddress;
   int memorysize;
} node;
#endif
