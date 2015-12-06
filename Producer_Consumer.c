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
#define MAX_TEXT 600 

void *thread_Producer(void *arg);
void *thread_Consumer(void *arg);

char message[] = "Im the Producr";
char message2[] = "Im the Checker";
char work_area[WORK_SIZE];
//int time_to_exit = 0;
pthread_mutex_t work_mutex;
pthread_t cons_thread[NUM_CONS];
pthread_t prod_thread;
pthread_t check_thread; 

pthread_attr_t thread_attr;


struct task_st {
	int pid;
	int static_prio;
	int prio ; 
	int execut_time;
	int time_slice;
	int accu_time_slice;
	int last_cpu;
	int slp_avg;
	char type[MAX_TEXT]; 
};

struct task_st buffer[20];


// create per-con ready queues 

struct cons_struct{
	struct task_st rq0[20];
	struct task_st rq1[20];
	struct task_st rq2[20];
};

struct cons_struct consq[5];

// create running queues 





int main()
{
	
	// local variables 
	int res; 
	
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


	// creat consumer threads and check that they were safely created 
	for(lots_of_threads = 0; lots_of_threads < NUM_CONS; lots_of_threads++) {
        res = pthread_create(&(cons_thread[lots_of_threads]), NULL, thread_Consumer, (void *)&lots_of_threads);
        if (res != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
    
	// create producer thread
	 res = pthread_create(&prod_thread,NULL,thread_Producer,(void *)message);
		if (res != 0) {
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}
		// create producer thread
	 res = pthread_create(&prod_thread,NULL,thread_Producer,(void *)message2);
		if (res != 0) {
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
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
	printf("All done\n");
    exit(EXIT_SUCCESS);
}

// this is the function of my Producer threads 
void *thread_Producer(void *arg) {
    int a,b,c,d,e =0;
    
    
    
    
    for(int i = 0; i <sizeof(buffer); i++){
		for(int j = 0; j <5; j++){
			// set the pid 
			buffer[i].pid = i;
			
			// now we are going to set the static priorety
			if(i<4){		
				buffer[i].static_prio = rand() %100;
			}	
			if(4<= i < 8){
				buffer[i].static_prio = rand() %100;
			}
			if(8<= i < 20){
				buffer[i].static_prio = (rand() %40) + 100;
			}
			// set the 
			buffer[i].prio = buffer[i].static_prio;
			
			buffer[i].execut_time = (rand() %45) +5;
			
			// Here the buffer is set into ready queue of destin cons
			if(j== 0){
				consq[0].rq0[a] = buffer[i]; 
				a++;
			}
			if(j== 1){
				consq[1].rq0[b] = buffer[i]; 
				b++;
			}
			if(j == 2){
				consq[2].rq0[c] = buffer[i]; ;
				c++;
			}
			if(j== 3){
				consq[3].rq0[d] = buffer[i]; 
				d++;
			}
			if(j== 4){
				consq[4].rq0[e] = buffer[i]; 
				d++;
			}
		}
		
		
	}
    
   
    printf("This is a producer thread Argument was");
  
    printf("Bye from %d\n");
    pthread_exit(NULL);
}

// This is the function of my Consumer threads 
void *thread_Consumer(void *arg) {    
     int cons_number = *(int *)arg;
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
        
    printf("%d", cons_number);
    
    if(buffer[cons_number].static_prio < 120){
			buffer[cons_number].time_slice = (140 - buffer[cons_number].static_prio)*20;
		
		}
		
	if(buffer[cons_number].static_prio > 120){
		buffer[cons_number].time_slice = (140 - buffer[cons_number].static_prio)*5;
	}
    
    usleep(20000);
	
	
	
	buffer[cons_number].accu_time_slice = buffer[cons_number].accu_time_slice + buffer[cons_number].time_slice;
	
    
    printf("This is A Consumer thread Argument was %d\n");
    printf("Bye from %d\n");
    pthread_exit(NULL);
}






