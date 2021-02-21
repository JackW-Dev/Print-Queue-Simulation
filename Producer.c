//Jack Walker
//Producer

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h> 


void main(int argc, char *argv[]){
	
	key_t flagKey, shmKey;
	int flagShmID, shmID, jobsPerCycle, maxJobs, i, priority, jobID, contFlag;
	char *flagShm, *shm, *end;
	FILE *fp;
	
	//Create or open text file in append mode
	fp = fopen("producerLog.txt", "a");
	
	//////////////////////////////////////////	
	//Set up and connect to shared memory space
	//Space is 8 bits in size as it will only ever hold 1 character
	flagKey = ftok("flag", 65);
	if(flagShmID = shmget(flagKey, 8, IPC_CREAT | 0666) < 0){
		printf("Error creating flag shared memory space -- Exiting application\n\r");
		fprintf(fp, "Error creating flag shared memory space -- Exiting application -- %ld\n\r", time(NULL));
		exit(1);
	}
	printf("Flag shared memory successfully created\n\r");
	fprintf(fp, "Flag shared memory successfully created -- %ld\n\r", time(NULL));
	
	if((flagShm = (char *)shmat(flagShmID, NULL, 0)) == (char*)-1){
			printf("Error connecting to flag shared memory space -- Exiting application\n\r");
			fprintf(fp, "Error connecting to flag shared memory space -- Exiting application -- %ld\n\r", time(NULL));
			exit(1);
	}	
	printf("Flag shared memory successfully connected to\n\r");
	fprintf(fp, "Flag shared memory successfully connected to -- %ld\n\r", time(NULL));
	//////////////////////////////////////////
	
	//////////////////////////////////////////	
	//Set up and connect to shared memory space
	//Space is larger as it will be for transferring data between prod and cons
	shmKey = ftok("data", 65);
	
	if(shmID = shmget(shmKey, 1024, IPC_CREAT | 0666) < 0){
		printf("Error creating dara shared memory space -- Exiting application\n\r");
		fprintf(fp, "Error creating data shared memory space -- Exiting application -- %ld\n\r", time(NULL));
		exit(1);
	}
	printf("Data shared memory successfully created\n\r");
	fprintf(fp, "Data shared memory successfully created -- %ld\n\r", time(NULL));
	
	if((shm = (char *)shmat(shmID, NULL, 0)) == (char*)-1){
			printf("Error connecting to data shared memory space -- Exiting application\n\r");
			fprintf(fp, "Error connecting to data shared memory space -- Exiting application -- %ld\n\r", time(NULL));
			exit(1);
	}
	printf("Data shared memory successfully connected to\n\r");
	fprintf(fp, "Data shared memory successfully connected to -- %ld\n\r", time(NULL));
	//////////////////////////////////////////
		
	while(1){
		//Loop production infinitely -- Will be broken out of with exit codes
		
		//Initialise flag in shared memory to show that control is with the producer
		*flagShm = 'P';
		
		//If flag is set to P, producer should process
		if(*flagShm == 'P'){
			fprintf(fp, "Producer has control -- %ld\n\r", time(NULL));			
			//Convert jobs per cycle and max jobs to integer values
			jobsPerCycle = strtol(argv[1], &end, 10);
			maxJobs = strtol(argv[2], &end, 10);
			
			if(jobsPerCycle < 1){
				printf("Jobs per cycle must be larger than 1 -- Exiting application\n\r");
				fprintf(fp, "Jobs per cycle < 1 -- Exiting Application -- %ld\n\r", time(NULL));
				//Detach and kill both shared memory spaces
				shmdt(*flagShm);
				shmdt(*shm);
				shmctl(flagShmID, IPC_RMID, NULL);
				shmctl(shmID, IPC_RMID, NULL);
				fclose(fp);
				//Exit application
				exit(0);
			}
			
			//Loop to produce print jobs
			for(i = 0; i < jobsPerCycle; i++){
				if(i % 5 == 0 && i > 0){
					//Will enter this every 5 jobs
					//User can select continue or exit at this point
					printf("%d Jobs created of %d\n\r", i, jobsPerCycle);
					if(continueProg() == 2){
						fprintf(fp, "User requested job production halt -- Exiting application -- %ld\n\r", time(NULL));
						i = jobsPerCycle;
						*flagShm = 'E';
						fprintf(fp, "Flag set to exit -- %ld\n\r", time(NULL));
						while(*flagShm == 'E'){
							//Do nothing whilst consumer kills all remaining jobs	
						}
						
						//Detach and kill both shared memory spaces
						shmdt(*flagShm);
						shmdt(*shm);
						shmctl(flagShmID, IPC_RMID, NULL);
						shmctl(shmID, IPC_RMID, NULL);
						fprintf(fp, "Shared memory detached and destroyed -- %ld\n\r", time(NULL));
						fprintf(fp, "Exiting application -- %ld\n\r", time(NULL));
						fclose(fp);
						//Exit application
						exit(0);
					}
				}
				if(getppid() == 0){
					//Will only enter here if the process is the parent
					//Ensures only requested number of jobs will be created
					priority = rand() % jobsPerCycle + 1;
					jobID = fork();
					if(getpid() > 0){
						//Only child processes will enter this
						while(1){
							//Infinite loop to halt child process
						}
					}else{
						//Add job details to shared memory
						*shm = priority;
						*shm++;
						*shm = jobID;
						*shm++;
					}
					fprintf(fp, "Job created -- PID: %d -- Priority: %d -- %ld\n\r", jobID, priority, time(NULL));
				}
			}
			//Set flag to C to signal ready for consumer
			*flagShm = 'C';
			fprintf(fp, "Flag set to consumer -- %ld\n\r", time(NULL));
		}else{
			//Wait and check shared memory flag again shortly
			sleep(1);
		}		
	}
}

int continueProg() {
    int choice, valid = 0, valueRead;
    char followChar;
    do {
        printf("Do you wish to continue creating jobs?\n\r");
        printf("1 - Yes\n\r2 - No\n\r");
        valueRead = scanf("%d%c", &choice, &followChar);
        if (valueRead == 2) {
            if (isspace(followChar) && choice > 0 && choice < 3) {
                /*Integer followed by whitespace*/
                valid = 1;
            } else {
                /*Integer followed by character*/
                printf("Only defined integer values will be accepted, please try again.\n\r");
                fflush(stdin);
            }
        } else if (valueRead == 1 && choice > 0 && choice < 3) {
            /*Integer followed by nothing*/
            valid = 1;
        } else {
            /*Not an integer*/
            printf("Only defined integer values will be accepted, please try again.\n\r");
            fflush(stdin);
        }
    } while (valid == 0);
    return choice;
}
