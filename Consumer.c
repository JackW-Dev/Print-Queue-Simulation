//Jack Walker
//Consumer

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h> 
#include <signal.h> 

struct jobNode{
	int priority;
	int jobID;
	struct jobNode *nextNode;
}*stNode;

void main(int argc, char* argv[]){
	key_t flagKey, shmKey;
	int flagShmID, shmID, jobsPerCycle, i, j, priority, jobID, contFlag;
	char *flagShm, *shm;
	struct jobNode *fnNode, *tmp;
	FILE *fp;
	
	//Create or open text file in append mode
	fp = fopen("consumerLog.txt", "a");
	
	//////////////////////////////////////////
	//Find and connect to flag shared memory
	flagKey = ftok("flag", 65);
		
	if((flagShmID = shmget(flagKey, 8, 0666)) < 0){
		printf("Error locating flag shared memory space -- Exiting application\n\r");
		fprintf(fp, "Error locating flag shared memory space -- Exiting application -- %ld\n\r", time(NULL));
	exit(1);
	}
	
	if((flagShm = (char *)shmat(flagShmID, NULL, 0)) == (char*)-1){
			printf("Error connecting to flag shared memory space -- Exiting application\n\r");
			fprintf(fp, "Error connecting to flag shared memory space -- Exiting application -- %ld\n\r", time(NULL));
			exit(1);
	}	
	printf("Flag shared memory successfully connected to\n\r");
	fprintf(fp, "Flag shared memory successfully connected to -- %ld\n\r", time(NULL));
	//////////////////////////////////////////
	
	//////////////////////////////////////////
	//Find and connect to data shared memory
	shmKey = ftok("data", 65);
	
	if((shmID = shmget(shmKey, 1024, 0666)) < 0){
		printf("Error locating data shared memory space -- Exiting application\n\r");
		fprintf(fp, "Error locating data shared memory space -- Exiting application -- %ld\n\r", time(NULL));
	exit(1);
	}
	
	if((shm = (char *)shmat(shmID, NULL, 0)) == (char*)-1){
			printf("Error connecting to flag shared memory space -- Exiting application\n\r");
			fprintf(fp, "Error connecting to flag shared memory space -- Exiting application -- %ld\n\r", time(NULL));
			exit(1);
	}	
	printf("Data shared memory successfully connected to\n\r");
	fprintf(fp, "Data shared memory successfully connected to -- %ld\n\r", time(NULL));
	//////////////////////////////////////////
	
	while(1){
		//Loop production infinitely -- Will be broken out of with exit codes
		
		//If flag is set to C, consumer should process
		if(*flagShm == 'C'){
			fprintf(fp, "Consumer has control -- %ld", time(NULL));			
			//Loop to remove jobs from shared memory
			for(i = 0; i < jobsPerCycle; i++){				
				if(i == 0){					
					//Start node
					stNode = (struct jobNode *)malloc(sizeof(struct jobNode));
					//Priority is 1st value in shared memory
					stNode->priority = shm;
					//Increment shared memory pointer to next value
					*shm++;
					//JobID is 2nd value in shared memory
					stNode->jobID = shm;
					//Increment shared memory to next value
					*shm++;
					//Set temporary node = start node
					tmp = stNode;	
					fprintf(fp, "Job removed from shared memory -- PID: %d -- Priority: %d -- %ld\n\r", tmp->jobID, tmp->priority, time(NULL));							
				}
				for(j = 0; j < jobsPerCycle-1; j++){
					//All other nodes created here
					fnNode = (struct jobNode *)malloc(sizeof(struct jobNode));
					//Priority is 1st value in shared memory
					fnNode->priority = shm;
					//Increment shared memory pointer to next value
					*shm++;
					//JobID is 2nd value in shared memory
					fnNode->jobID = shm;
					//Increment shared memory pointer to next value
					*shm++;
					//Set nest node to NULL
					fnNode->nextNode = NULL;
					//Set the next node pointer of temp to be the current node (Double linked list) 
					tmp->nextNode = fnNode; // links previous node i.e. tmp to the fnNode
					//Set temp node to be the current node
                	tmp = tmp->nextNode;
                	fprintf(fp, "Job removed from shared memory -- PID: %d -- Priority: %d -- %ld\n\r", tmp->jobID, tmp->priority, time(NULL));
				}								
			}
			//Sort linked list
			fprintf(fp, "Sorting linked list of nodes based on priority -- %ld\n\r", time(NULL));
			bubbleSort(stNode);
			while(stNode->nextNode != NULL){
				//Nodes should now be organised and ready to be killed in order or priority
				printf("Killing job %d", stNode->jobID);
				kill(stNode->jobID, SIGKILL);
				fprintf(fp, "Job killed -- PID: %d -- Priority: %d -- %ld", stNode->jobID, stNode->priority, time(NULL));
				stNode = stNode->nextNode;
			}
			flagShm = 'P';
		}else if(*flagShm == 'E'){
			fprintf(fp, "Exit flag found -- Killing all jobs -- %ld", time(NULL));
			//Flag is set to exit
			//Loop to remove jobs from shared memory
			for(i = 0; i < jobsPerCycle; i++){				
				if(i == 0){
					//Start node
					stNode = (struct jobNode *)malloc(sizeof(struct jobNode));
					//Priority is 1st value in shared memory
					stNode->priority = shm;
					//Increment shared memory pointer to next value
					*shm++;
					//JobID is 2nd value in shared memory
					stNode->jobID = shm;
					//Increment shared memory to next value
					*shm++;
					//Set temporary node = start node
					tmp = stNode;								
				}
				for(j = 0; j < jobsPerCycle-1; j++){
					//All other nodes created here
					fnNode = (struct jobNode *)malloc(sizeof(struct jobNode));
					//Priority is 1st value in shared memory
					fnNode->priority = shm;
					//Increment shared memory pointer to next value
					*shm++;
					//JobID is 2nd value in shared memory
					fnNode->jobID = shm;
					//Increment shared memory pointer to next value
					*shm++;
					//Set nest node to NULL
					fnNode->nextNode = NULL;
					//Set the next node pointer of temp to be the current node (Double linked list) 
					tmp->nextNode = fnNode; // links previous node i.e. tmp to the fnNode
					//Set temp node to be the current node
                	tmp = tmp->nextNode;
				}								
			}
			//Sort linked list
			bubbleSort(stNode);	
			while(stNode->nextNode != NULL){
				//Nodes should now be organised and ready to be killed in order or priority
				printf("Killing job %d", stNode->jobID);
				kill(stNode->jobID, SIGKILL);
				fprintf(fp, "Job killed -- PID: %d -- Priority: %d -- %ld", stNode->jobID, stNode->priority, time(NULL));
				stNode = stNode->nextNode;
			}
			fprintf(fp, "All jobs killed -- Exiting application -- %ld", time(NULL));
			printf("All jobs killed -- Exiting application\n\r");
			*flagShm = 'P';
			shmdt(flagShm);
			shmdt(shm);
			fprintf(fp, "Shared memory detached from -- %ld\n\r", time(NULL));
			fclose(fp);
			exit(0);
		}else{
			//Wait and check shared memory flag again shortly
			sleep(1);
		}
		
	}
}

//Simple sort to organise linked list by priority
void bubbleSort(struct jobNode *start) 
{ 
    int swapped, i; 
    struct jobNode *nodePtr; 
    struct jobNode *endPtr = NULL; 
  
    //Check for empty list
    if (start == NULL) 
        return;   
    do
    { 
        swapped = 0; 
        nodePtr = start; 
  
        while (nodePtr->nextNode != endPtr) 
        { 
            if (nodePtr->priority > nodePtr->nextNode->priority) 
            {  
                swap(nodePtr, nodePtr->nextNode); 
                swapped = 1; 
            } 
            nodePtr = nodePtr->nextNode; 
        } 
        endPtr = nodePtr; 
    } 
    while (swapped); 
} 
  
//Swaps jobID and priority of 2 nodes
void swap(struct jobNode *a, struct jobNode *b) 
{ 
	int tempPriority, tempJobID;
	
    tempJobID = a->jobID;
	tempPriority = a->priority;
    a->jobID = b->jobID;
	a->priority = b->priority; 
    b->jobID = tempJobID;
	b->priority = tempPriority; 
} 

