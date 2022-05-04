#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

void cont(int signum);
int prevclk;

int main(int argc, char *argv[])
{
    initClk();
    signal(SIGCONT,cont);

    //TODO The process needs to get the remaining time from somewhere
    remainingtime = atoi(argv[0]);

   // printf("time at beginning for process %d: %d with runtime %d\n",getpid(),getClk(),atoi(argv[0]));


    for (remainingtime;remainingtime>0;remainingtime--){
        prevclk=getClk();
        while (prevclk==getClk()) {}
       // printf("clk changed at time %d\n",getClk());
    }


  // printf("time at ending for process %d : %d\n",getpid(),getClk());

    kill(getppid(),SIGUSR1);

    destroyClk(0);

    return 0;
}

void cont(int signum)
{
    //printf("sigcont for process %d at %d remaining time %d\n",getpid(),getClk(),remainingtime);
    prevclk=getClk();
    signal(SIGCONT,cont);
}