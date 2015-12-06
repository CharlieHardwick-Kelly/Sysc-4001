/*
 * Producer_Consumer.c
 * 
 * Copyright 2015 Charlie Hardwick-Kelly <charliehardwickkelly@cb5109-24-l>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 */




// here are the includes needed for this assignment 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>


// here are the defines needed for this assignment 
#define NUM_CONS 4
#define WORK_SIZE 1024
#define BUFFER_SIZE 20
#define MAX_SLEEP_AVG 10

// function initialization
void *thread_Producer(void *arg);
void *thread_Consumer(void *arg);
void find_Next_Process(int cons_number);
int find_highest_prio(int cons_number, int queue, int start, int end);
int remove_from_queue(int cons_number, int queue, int index, int end);

// Here we have the global variables needed to run the program 
char message[] = "Im the Producr";
char message2[] = "Im the Checker";
char work_area[WORK_SIZE];
pthread_mutex_t work_mutex;
pthread_t cons_thread[NUM_CONS];
pthread_t prod_thread;
pthread_t check_thread; 
pthread_attr_t thread_attr;
int dead_thread =666;

// this is the process structure for this program
struct task_st {
	int pid;
	int static_prio;
	int prio ; 
	int execut_time;
	int time_slice;
	int accu_time_slice;
	int last_cpu;
	int slp_avg;
	int type; 
	int tod;
};

struct task_st buffer[20];


// These are the ready queues for all the consumer threads 
struct cons_struct{
	struct task_st rq0[25];
	struct task_st rq1[25];
	struct task_st rq2[25];

	int rq0L;
	int rq1L;
	int rq2L;
};

struct cons_struct consq[4];


int main()
{
	// local variables 
	int res; 
	// set the buffer counter values to the intial value of 0 
	consq[0].rq0L=0;
	consq[0].rq1L=0;
	consq[0].rq2L=0;

	consq[1].rq0L=0;
	consq[1].rq1L=0;
	consq[1].rq2L=0;

	consq[2].rq0L=0;
	consq[2].rq1L=0;
	consq[2].rq2L=0;

	consq[3].rq0L=0;
	consq[3].rq1L=0;
	consq[3].rq2L=0;

	
	// creat thread variables 
	void *thread_result;
	int lots_of_threads;
	int producers;
	int consumers;
	
	// priority values 
	int max_priority;
    int min_priority;
    struct sched_param scheduling_value;
	
	// create thread attributes 
	res = pthread_attr_init(&thread_attr);
	if (res != 0) {
        perror("Attribute creation failed");
        exit(EXIT_FAILURE);
    }
	// set thread sched poicy
	res = pthread_attr_setschedpolicy(&thread_attr, SCHED_OTHER);
    if (res != 0) {
        perror("Setting schedpolicy failed");
        exit(EXIT_FAILURE);
    }
    // set thread detach state 
    res = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if (res != 0) {
        perror("Setting detached attribute failed");
        exit(EXIT_FAILURE);
	}
	max_priority = sched_get_priority_max(SCHED_OTHER);
    min_priority = sched_get_priority_min(SCHED_OTHER);
    scheduling_value.sched_priority = min_priority;
    res = pthread_attr_setschedparam(&thread_attr, &scheduling_value);
    if (res != 0) {
        perror("Setting schedpolicy failed");
        exit(EXIT_FAILURE);
    }
	// Create  mutex and check that it safely created 
	 res = pthread_mutex_init(&work_mutex, NULL);
    if (res != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
	}
			// create producer thread
	 res = pthread_create(&prod_thread,NULL,thread_Producer,(void *)message2);
		if (res != 0) {
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}

	// create consumer threads and check that they were safely created 
	for(lots_of_threads = 0; lots_of_threads < NUM_CONS; lots_of_threads++) {
        res = pthread_create(&(cons_thread[lots_of_threads]), NULL, thread_Consumer, (void *)&lots_of_threads);
        if (res != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
    	
   
     printf("Waiting for threads to finish...\n");
    for(lots_of_threads = NUM_CONS - 1; lots_of_threads >= 0; lots_of_threads--) {
        res = pthread_join(cons_thread[lots_of_threads], &thread_result);
        if (res == 0) {
            printf("Picked up a thread\n");
        }
        else {
            perror("pthread_join failed");
        }
    }
    exit(EXIT_SUCCESS);
}

// this is the function of my Producer threads 
void *thread_Producer(void *arg) {
     	int a = 0; 
    	int b = 0; 
    	int c = 0; 
    	int d = 0;
    	int j = 0;

   		pthread_mutex_lock(&work_mutex);
    	for(int i = 0; i < BUFFER_SIZE ; i++){
    	printf("Produced : %d\n", i);
    		if(j == 4){
    			j=0;
    		}
		
			// set the pid 
			buffer[i].pid = i;
			//buffer[i].tod = gettimeofday();
			// now we are going to set the static priorety
			// type 1 is FIFO 
			if(i<4){		
				buffer[i].static_prio = rand() %100;
				buffer[i].type = 1;
			}	
			// type 2 is round robin  1/5 
			if(4<= i && i< 8){
				buffer[i].static_prio = rand() %100;
				buffer[i].type = 2;
			}
			// type 3 is normal 3/5
			if(8<= i && i < 20){
				buffer[i].static_prio = (rand() %40) + 100;
				buffer[i].type = 3;
			}

			buffer[i].prio = buffer[i].static_prio;
			
			buffer[i].execut_time = (rand() %45) +5;
			// Here the buffer is set into ready queue of destin cons


			//ccalculate the time slice for the thread      
  		  	if(buffer[i].static_prio < 120){
				buffer[i].time_slice = (140 - buffer[i].static_prio)*20;
			}
		
			if(buffer[i].static_prio > 120){
				buffer[i].time_slice = (140 - buffer[i].static_prio)*5;
			}



			// place the created thread into a consumers q 
			// if a normal execution move into the seccond rq of the consumer
			if(j== 0){
				if(buffer[i].type == 3){
					consq[0].rq1[a] = buffer[i]; 
					consq[0].rq1L++; 
				}
				else{
					consq[0].rq0[a] = buffer[i]; 
					consq[0].rq0[a] = buffer[i]; 
					consq[0].rq0L ++; 
				}
				a++;
			}
			if(j== 1){
				if(buffer[i].type == 3){
					consq[1].rq1[b] = buffer[i]; 
					consq[1].rq1L++; 
				}
				else{
					consq[1].rq0[b] = buffer[i]; 
					consq[1].rq0[b] = buffer[i]; 
					consq[1].rq0L ++; 
				}
				b++;
			}
			if(j == 2){
				if(buffer[i].type == 3){
					consq[2].rq1[a] = buffer[i]; 
					consq[2].rq1L++; 
				}
				else{consq[2].rq0[c] = buffer[i]; 
					consq[2].rq0[c] = buffer[i]; 
					consq[2].rq0L ++; 
				}
				c++;
			}
			if(j == 3){
				if(buffer[i].type == 3){
					consq[3].rq1[d] = buffer[i]; 
					consq[3].rq1L++; 
				}
				else{consq[3].rq0[d] = buffer[i]; 
					consq[3].rq0[d] = buffer[i]; 
					consq[3].rq0L ++; 
				}
			}
			j++;
		}
		
		pthread_mutex_unlock(&work_mutex);
		
	

	printf("Threads succesfully produced!\n");
   	
    pthread_exit(NULL);
}

// This is the function of my Consumer threads 
void *thread_Consumer(void *arg) {    
     int cons_number = *(int *)arg;

     printf("Starting consumer %d\n",cons_number);




	sleep(1);
	// here need to call function determine which process needs to be run next from the queue 
	pthread_mutex_lock(&work_mutex);
	find_Next_Process(cons_number);

	pthread_mutex_unlock(&work_mutex);

        
    printf("%d", cons_number);
    
    //usleep(20000);
	
	
	
	buffer[cons_number].accu_time_slice = buffer[cons_number].accu_time_slice + buffer[cons_number].time_slice;
	
    
  //  printf("This is A Consumer thread Argument was \n");
  //  printf("Bye from \n");
    pthread_exit(NULL);
}



void find_Next_Process(int cons_number){
	int plh0 = consq[cons_number].rq0L;
	int plh0c = 0; 
	int plh1 = consq[cons_number].rq1L;
	int plh1c = 0;
	int plh2 = consq[cons_number].rq2L;
	int plh2c = 0;
	int highest_prio;


	printf("plh0 %d\n",plh0);

	// Find if process in ready queue 0
	while(1){
		// check that the queue is not empty 
		if(plh0 != plh0c){
			
			// now need to find the highest priorety in the queue 0 
			printf("plh0 before highest_prio: %d\n", plh0);
			highest_prio = find_highest_prio(cons_number, 0 ,plh0c, plh0);
			if(highest_prio == -99 ){
				perror("Cant find highest priorety");
				exit(EXIT_FAILURE);
			}
			printf("highest_prio loc: %d\n", highest_prio);
			printf("in here1\n");
			printf("highest_prio_type: %d\n",consq[cons_number].rq0[highest_prio].type);
			
			if(consq[cons_number].rq0[highest_prio].type == 1){ 
				// Type FIFO there forw execute to completion
				usleep(consq[cons_number].rq0[highest_prio].execut_time);
				printf("The FIFO is complete\n");
				consq[cons_number].rq0[highest_prio].pid = dead_thread;

				if(consq[cons_number].rq0[highest_prio].pid == dead_thread){

					
					plh0 = remove_from_queue(cons_number,0,highest_prio,plh0);
					if(plh0 == -99){
						perror("node removal failed");
						exit(EXIT_FAILURE);
					}
					printf("FIFO succesfully removed!");
				

				}
				

			}

			if(consq[cons_number].rq0[highest_prio].type == 2){
				// Type RR there for execute for time quant
				printf(" in here 2 mahhhhhn \n" );

				usleep(consq[cons_number].rq0[highest_prio].time_slice); 
				consq[cons_number].rq0[highest_prio].accu_time_slice += consq[cons_number].rq0[highest_prio].time_slice;
				if(consq[cons_number].rq0[highest_prio].accu_time_slice >= consq[cons_number].rq0[highest_prio].execut_time){
					consq[cons_number].rq0[highest_prio].pid = dead_thread;


					plh0 = remove_from_queue(cons_number,0,highest_prio,plh0);
					if(plh0 == -99){
						perror("node removal failed");
						exit(EXIT_FAILURE);
					}

					printf("The RR process has succesfully been removed : %d\n",plh0);

				}





			}

			if(consq[cons_number].rq2[highest_prio].type == 3){
				// if type normaal do some stuff 
				printf(" in here 3");
			}

		}

		// Find if process in ready queue 0
		if(plh1 != 0){
			
		}

		// Find if process in ready queue 0
		if(plh2 != 0){
	

		}
		//printf(" No threads to execute \n");
		//exit(EXIT_SUCCESS);
	}
	



}

// returns the location of the highest priorety of a given queue 
int find_highest_prio(int cons_number, int queue, int start, int end){
	int location; 
	int highest = 0;

	if(queue ==0){
		
		for (int i = start; i < end; i++){
			if(consq[cons_number].rq0[i].prio > highest){
				highest = consq[cons_number].rq0[i].prio;
				location = i; 
			}
			return location; 
		}
	}

	if(queue ==1){
		// complete
	}
	if(queue ==2){
		// complete
	}





	return -99;
}

int remove_from_queue(int cons_number, int queue, int index, int end){

	if(queue == 0){
		for (int i = index; i < end -1 ; i++){
				consq[cons_number].rq0[i] = consq[cons_number].rq0[i+1];
		}	
		return end - 1;

	}
	if(queue == 1){
		// complete
	}
	if(queue == 2){
		// complete
	}
	return -99;
}



