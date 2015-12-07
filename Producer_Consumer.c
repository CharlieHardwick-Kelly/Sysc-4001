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
#include <sys/time.h>

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
	int time_remain; 
	int time_slice;
	int accu_time_slice;
	int last_cpu;
	int slp_avg;
	int type; 
	struct timeval arrival_time;
	struct timeval sleep_time; 
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
			


			//set the execution time 
			buffer[i].execut_time = (rand() %45) +5;
			buffer[i].time_remain = buffer[i].execut_time;
			// Here the buffer is set into ready queue of destin cons


			//ccalculate the time slice for the thread      
  		  	if(buffer[i].static_prio < 120){
				buffer[i].time_slice = (140 - buffer[i].static_prio)*20;
			}
		
			if(buffer[i].static_prio > 120){
				buffer[i].time_slice = (140 - buffer[i].static_prio)*5;
			}

			// set the time of day 

			gettimeofday(&buffer[i].arrival_time, NULL); 
			gettimeofday(&buffer[i].sleep_time, NULL); 

			// place the created thread into a consumers q 
			// if a normal execution move into the seccond rq of the consumer
			if(j== 0){
				if(buffer[i].type == 3){
					consq[0].rq1[consq[0].rq1L] = buffer[i]; 
					consq[0].rq1L++; 
				}
				else{
					consq[0].rq0[consq[0].rq0L] = buffer[i]; 
					consq[0].rq0L ++; 
				}
				a++;
			}
			if(j== 1){
				if(buffer[i].type == 3){
					consq[1].rq1[consq[1].rq1L] = buffer[i]; 
					consq[1].rq1L++; 
				}
				else{
					consq[1].rq0[consq[1].rq0L] = buffer[i]; 
					consq[1].rq0L ++; 
				}
				b++;
			}
			if(j == 2){
				if(buffer[i].type == 3){
					consq[2].rq1[consq[2].rq1L] = buffer[i]; 
					consq[2].rq1L++; 
				}
				else{
					consq[2].rq0[consq[2].rq0L] = buffer[i]; 
					consq[2].rq0L ++; 
				}
				c++;
			}
			if(j == 3){
				if(buffer[i].type == 3){
					consq[3].rq1[consq[3].rq1L] = buffer[i]; 
					consq[3].rq1L++; 
				}
				else{
					consq[3].rq0[consq[3].rq0L] = buffer[i];  
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
	
    
  //  printf("This is A Consumer thread Argument was \n");
  //  printf("Bye from \n");
    pthread_exit(NULL);
}


// returns the location of the highest priorety of a given queue 
int find_highest_prio(int cons_number, int queue){
	int location = 0; 
	int highest = 10000;
	if(queue ==0){
		for (int i = 0; i < 25; i++){
			if(consq[cons_number].rq0[i].prio < highest){
				highest = consq[cons_number].rq0[i].prio;
				location = i; 
			}
			if(highest == 10000){

				return 66;
			}
			return location; 
		}
	}

	if(queue ==1){
			for (int i = 0; i < 25; i++){
			if(consq[cons_number].rq1[i].prio < highest){
				highest = consq[cons_number].rq1[i].prio;
				location = i; 
			}
			if(highest == 10000){

				return 66;
			}
			return location; 
		}
	}
	if(queue ==2){
			for (int i = 0; i < 25; i++){
			if(consq[cons_number].rq2[i].prio < highest){
				highest = consq[cons_number].rq2[i].prio;
				location = i; 
			}
			if(highest == 10000){

				return 66;
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


// this process essentially runs the program for the given consumer
void find_Next_ProcessV2(int cons_number){
	int plh0 = consq[cons_number].rq0L;
	int plh1 = consq[cons_number].rq1L;
	int plh2 = consq[cons_number].rq2L;


	int hpRq0;
	int hpRq1;
	int hpRq2;
	int curq;
	int highest_prio;

	// need to check if there is anything in rq0 
	
	
	int ongoing = 1;
	while(ongoing){
		if(plh0 == 0){
			printf("killed 1\n");
			ongoing = 0;
			
		}

		// get the highest priorety in the queue 
		hpRq0 = find_highest_prio(cons_number,0);
		if(hpRq0 == -99 ){
			perror("Cant find highest  rq0");
			exit(EXIT_FAILURE);
		}
		if(hpRq0 == 66){
			printf("rq %d is empty \n ", 0);
			hpRq0 =0 ;
		}
		 hpRq1 = find_highest_prio(cons_number,1);
		if(hpRq1 == -99 ){ 
			perror("Cant find highest priorety rq1");
			exit(EXIT_FAILURE);
		}
		if(hpRq1 == 66){
			printf("rq %d is empty \n ", 1);
			hpRq1 =0 ;
		}

		 hpRq2 = find_highest_prio(cons_number,2);
		if(hpRq2 == -99 ){
			perror("Cant find highest priorety rq2");
			exit(EXIT_FAILURE);
		}
		if(hpRq2 == 66){
			printf("rq %d is empty\n ", 2);
			hpRq2 =0 ;
		}

			// determine which of the highest priorety values is best
		 if(hpRq0 < hpRq1 && hpRq0 < hpRq2){
		 	highest_prio = hpRq0;
		 	curq = 0;

		 }
		
		 if(hpRq1 < hpRq0 && hpRq1 < hpRq2){
		 	highest_prio = hpRq1;
		 	curq = 1;
		 }

		 if(hpRq2 < hpRq1 && hpRq2 <hpRq0){
		 	highest_prio = hpRq2;
		 	curq = 2;
		 }	


		// check if the highest priorety is of type fifo 	
		if(curq==0){
				
			if(consq[cons_number].rq0[highest_prio].type == 1){
				usleep(consq[cons_number].rq0[highest_prio].execut_time);

				// calculate the turnaround time 
				struct timeval finished ; 
				gettimeofday(&finished,NULL);

				double turnarround = (double)(consq[cons_number].rq1[highest_prio].arrival_time.tv_sec - finished.tv_sec);


				printf("FIFO succesfully removed! pid: %d Turnarround time: %f\n ", consq[cons_number].rq0[highest_prio].pid, turnarround);
				remove_from_queue(cons_number,0,highest_prio);
				
				plh0--; 
				if(plh0 == 0 ){
					ongoing = 0; 
				}
				
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
					


					// determine the turnaround time using current time and start timeval
					struct timeval finished ; 
					gettimeofday(&finished,NULL);

					double turnarround = (double)(consq[cons_number].rq1[highest_prio].arrival_time.tv_sec - finished.tv_sec);

					printf(" RR succesfully removed pid: %d Turnarround time: %f \n", consq[cons_number].rq0[highest_prio].pid, turnarround );

					remove_from_queue(cons_number,0,highest_prio);
					plh0--;

					if(plh0 == 0 ){
					ongoing = 0; 
					}
				}
				else{
					// determine time remaining 
					consq[cons_number].rq0[highest_prio].time_remain -= consq[cons_number].rq0[highest_prio].time_slice;
					
					printf("RR pid: %d still has %d  remaining \n", consq[cons_number].rq0[highest_prio].pid, 	consq[cons_number].rq0[highest_prio].time_remain);
				}

			}

		}
		else{
			//  if not of type fifo or RR is of type Normal 
			printf(" Type is normal \n");

			// break down into rq1 and rq0

			if(curq == 1){
				if(consq[cons_number].rq1[highest_prio].time_remain < consq[cons_number].rq0[highest_prio].time_slice){

					usleep(consq[cons_number].rq1[highest_prio].time_remain);
					// determine the turnarround time 

					struct timeval finished ; 
					gettimeofday(&finished,NULL);

					double turnarround = (double)(consq[cons_number].rq1[highest_prio].arrival_time.tv_sec - finished.tv_sec);



					printf("Normal succesfully removed pid: %d Turnarround time: %f \n", consq[cons_number].rq1[highest_prio].pid, turnarround );
				}
				else{
					int prev_prio  = consq[cons_number].rq1[highest_prio].prio; 
					usleep(consq[cons_number].rq1[highest_prio].time_slice);



					// find the amount of tics in time slept 

					struct timeval slp_end ;
					gettimeofday(&slp_end,NULL);

					double sleep_tiks  = (double)(slp_end.tv_sec  - consq[cons_number].rq1.sleep_time.tv_sec);

					



				}




				}

			}


			if(curq == 2){


			}

			// check if time remaining is less than time slice 

			







			break;




		}
	}


	// now moving on to "normal type"






}




