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
void find_Next_ProcessV2(int cons_number);	
int find_highest_prio(int cons_number, int queue);
void remove_from_queue(int cons_number, int queue,int index);

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
	int prio; 
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
		sleep(2);

	// create consumer threads and check that they were safely created 
	for(lots_of_threads = 0; lots_of_threads < NUM_CONS; lots_of_threads++) {
        res = pthread_create(&(cons_thread[lots_of_threads]), NULL, thread_Consumer, (void *)&lots_of_threads);
        sleep(1);
        if (res != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
	sleep(10);    	
   
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

			// set the initial avg to 0 
			buffer[i].slp_avg = 0 ;
			
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
					consq[1].rq0L ++; 
				}
				b++;
			}
			if(j == 2){
				if(buffer[i].type == 3){
					consq[2].rq1[c] = buffer[i]; 
					consq[2].rq1L++; 
				}
				else{
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
				else{
					consq[3].rq0[d] = buffer[i];  
					consq[3].rq0L ++; 
				}
				d++;
			}
			j++;
		}
		printf(" check : %d %d %d %d \n",consq[0].rq0L,consq[1].rq0L,consq[2].rq0L,consq[3].rq0L);
		printf(" check : %d %d %d %d \n",a,b,c,d);
		pthread_mutex_unlock(&work_mutex);
		
	

	printf("Threads succesfully produced!\n");
   	
    pthread_exit(NULL);
}

// This is the function of my Consumer threads 
void *thread_Consumer(void *arg) {    
     int cons_number = *(int *)arg;
     printf("Starting consumer %d\n",cons_number);




	
	// here need to call function determine which process needs to be run next from the queue 
	pthread_mutex_lock(&work_mutex);
	find_Next_ProcessV2(cons_number);
	pthread_mutex_unlock(&work_mutex);

        
    printf("%d\n", cons_number);
    
    //usleep(20000);
	
	
	
	buffer[cons_number].accu_time_slice = buffer[cons_number].accu_time_slice + buffer[cons_number].time_slice;
	
    
  //  printf("This is A Consumer thread Argument was \n");
  //  printf("Bye from \n");
    pthread_exit(NULL);
}


// returns the location of the highest priorety of a given queue 
int find_highest_prio(int cons_number, int queue){
	int location = 0; 
	int highest =  0;
	if(queue ==0){
		for (int i = 0; i < 25; i++){
			if(consq[cons_number].rq0[i].prio > highest){
				highest = consq[cons_number].rq0[i].prio;
				location = i; 
			}
			if(highest == 0){

				return -99;
			}
			return location; 
		}
	}

	if(queue ==1){
			for (int i = 0; i < 25; i++){
			if(consq[cons_number].rq1[i].prio > highest){
				highest = consq[cons_number].rq1[i].prio;
				location = i; 
			}
			if(highest == 0){

				return -99;
			}
			return location; 
		}
	}
	if(queue ==2){
			for (int i = 0; i < 25; i++){
			if(consq[cons_number].rq2[i].prio > highest){
				highest = consq[cons_number].rq2[i].prio;
				location = i; 
			}
			if(highest == 0){

				return -99;
			}
			return location; 
		}
	}

	return -99;
}





// remove a variable from a given consumer, process queue and index 
void remove_from_queue(int cons_number, int queue, int index){

	if(queue == 0){
		for (int i = index; i < 24 ; i++){
				consq[cons_number].rq0[i] = consq[cons_number].rq0[i+1];
		}	
	}
	if(queue == 1){
		for (int i = index; i < 24 ; i++){
				consq[cons_number].rq1[i] = consq[cons_number].rq1[i+1];
		}	
	}
	if(queue == 2){
		for (int i = index; i < 24 ; i++){
				consq[cons_number].rq2[i] = consq[cons_number].rq2[i+1];
		}	
	}
	
}

void find_Next_ProcessV2(int cons_number){
	int plh0 = consq[cons_number].rq0L;
	int plh0c = 0; 
	int plh1 = consq[cons_number].rq1L;
	int plh1c = 0;
	int plh2 = consq[cons_number].rq2L;
	int plh2c = 0;
	int highest_prio;

	// need to check if there is anything in rq0 
	
	
	int ongoing = 1;
	while(ongoing){
		if(plh0 == 0){
			printf("killed 1\n");
			ongoing = 0;
			
		}

		// get the highest priorety in the queue 
		highest_prio = find_highest_prio(cons_number,0);

			


			if(highest_prio == -99 ){
				perror("Cant find highest priorety");
				exit(EXIT_FAILURE);
			}
			if(highest_prio == 66){
				break; 
			}
			highest_prio = 0; 
		// check if the highest priorety is of type fifo 	

			
		if(consq[cons_number].rq0[highest_prio].type == 1){
			usleep(consq[cons_number].rq0[highest_prio].execut_time);
			printf("FIFO succesfully removed! pid: %d\n", consq[cons_number].rq0[highest_prio].pid);
			remove_from_queue(cons_number,0,highest_prio);
			
			plh0--; 
			if(plh0 == 0 ){
				ongoing = 0; 
			}
			
			// the queue is empty 
			
		}
		// check if highest prio is RR
		if(consq[cons_number].rq0[highest_prio].type == 2){
			// sleep for time slice
			usleep(consq[cons_number].rq0[highest_prio].time_slice); 
			// update total time slice 
			consq[cons_number].rq0[highest_prio].accu_time_slice += consq[cons_number].rq0[highest_prio].time_slice;

			// check  if RR is completed  
			if(consq[cons_number].rq0[highest_prio].accu_time_slice > consq[cons_number].rq0[highest_prio].execut_time){
				
				// set the prio to 0 
				
				printf(" RR succesfully removed pid: %d \n", consq[cons_number].rq0[highest_prio].pid );
				remove_from_queue(cons_number,0,highest_prio);
				plh0--;

				if(plh0 == 0 ){
				ongoing = 0; 
				}
			}

		}
		else{
			break;
		}
	}


	// now moving on to "normal type"






}




