#include "queue.h"
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <readline/readline.h>
#include "customer_info.h"


#define NUM_CLERK 5 //Variable for clerks
#define MAX_CUST 30 // announcment said at most 30 customers


/* global variables */
 
struct timeval init_time; // Used for time calculations in calculate_time()

struct customer_info file_storage[256]; //Where we read and store the data from text file
int num_cust = 0;  // Length of customer
int num_served;  // Number of customers served, protected by mutex served_customers
struct Queue* business_queue; // Queue for the people in business class, protectewd by mutex qbus_mutex
struct Queue* standard_queue; // Queue for customers in economy class, protected by mutex qstan_mutex
int business_queue_len = 0; // 
int standard_queue_len = 0; // These two lengths are length of current queue, indirectly covered by enqueue mutex
int business_queue_max =0;
int standard_queue_max = 0; // Total max size
int cust_to_clerk[5]; // To remember which clerk helped what customer
int clerk_num;
double standard_wait = 0; // For the ending average time calculations
double business_wait = 0;
double total_wait = 0;

pthread_mutex_t qbus_mutex;
pthread_mutex_t qstan_mutex;
pthread_mutex_t enqueue_mutex;
pthread_mutex_t clerk_mutex[NUM_CLERK]; // Protect individual clerk threads
pthread_mutex_t cust_mutex[MAX_CUST]; // Protect individual customer threads
pthread_mutex_t build_cust; 
pthread_mutex_t build_clerk;

pthread_mutex_t served_customers;

pthread_cond_t clerk_convar[NUM_CLERK]; // Used to tell a clerk when they can move onto next customer
pthread_cond_t cust_convar[MAX_CUST];  // Used to wake individual customer threads that a clerk is available

pthread_mutex_t business_time;
pthread_mutex_t standard_time;

double calculate_time(struct timeval init_time){ // Calculates the time spent between durations
	struct timeval cur_time;
	double start_seconds;
	double cur_seconds;
	start_seconds = (init_time.tv_sec + (double) init_time.tv_usec / 1000000);
	gettimeofday(&cur_time, NULL);
	cur_seconds = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
	return(cur_seconds-start_seconds);
}


void *clerk_entry(void * clerkNum){ // This is the main thread for each clerk
	int clerk_id = *(int*)clerkNum;
	int queue_id;

	usleep(10);
	while(1){
		usleep(50);
		if (num_served==num_cust){ // When num_served==num_cust, we are done and can exit
			pthread_exit(NULL);
		}

		if(business_queue_len>0){
			pthread_mutex_lock(&served_customers);
			num_served++;
			pthread_mutex_unlock(&served_customers);

			pthread_mutex_lock(&qbus_mutex);
			//struct customer_info temp_cust = dequeue(business_queue);
			int temp_id = dequeue(business_queue);
			
			
			business_queue_len --;
			
			pthread_mutex_unlock(&qbus_mutex);

			cust_to_clerk[clerk_id] = temp_id;

			pthread_mutex_lock(&clerk_mutex[clerk_id]);
			pthread_cond_broadcast(&cust_convar[temp_id]);
			pthread_cond_wait(&clerk_convar[clerk_id] ,&clerk_mutex[clerk_id]);
			pthread_mutex_unlock(&clerk_mutex[clerk_id]);
			//break;
			

		}else if(standard_queue_len>0){
			pthread_mutex_lock(&served_customers);
			num_served++;
			pthread_mutex_unlock(&served_customers);

			pthread_mutex_lock(&qstan_mutex);
			//struct customer_info temp_cust = dequeue(business_queue);
			int temp_id = dequeue(standard_queue);
			standard_queue_len--;
			pthread_mutex_unlock(&qstan_mutex);

			cust_to_clerk[clerk_id] = temp_id;

			pthread_mutex_lock(&clerk_mutex[clerk_id]);
			pthread_cond_broadcast(&cust_convar[temp_id]);
			pthread_cond_wait(&clerk_convar[clerk_id], &clerk_mutex[clerk_id]);
			pthread_mutex_unlock(&clerk_mutex[clerk_id]);
			//break;
			
		}else{
			
		}
		usleep(140);
	}
}

void *customer_entry(void * customer_data){ 			// This is the main thread for each customer
	struct customer_info *customer = (struct customer_info*) customer_data;
	if (customer->class_type == 1){
		pthread_mutex_lock(&enqueue_mutex);
		int* temp_user_id = malloc(sizeof(int));
		temp_user_id = customer->user_id;
		enqueue(business_queue, temp_user_id);
		business_queue_len++;
		business_queue_max++;
		
		fprintf(stdout, "A customer arrives: customer ID %2d. \n", customer->user_id);
		fprintf(stdout, "A customer enters Business Queue. customer ID:%2d. The Business Queue is now of length %d \n", customer->user_id, business_queue_len);
		pthread_mutex_unlock(&enqueue_mutex);
	} else if (customer->class_type == 0){
		pthread_mutex_lock(&enqueue_mutex);
		int* temp_user_id = malloc(sizeof(int));
		temp_user_id = customer->user_id; 
		enqueue(standard_queue, temp_user_id);
		standard_queue_len++;
		standard_queue_max++;
		fprintf(stdout, "A customer arrives: customer ID %2d. \n", customer->user_id);
		fprintf(stdout, "A customer enters Economy Queue. customer ID:%2d. The Economy Queue is now of length %d\n", customer->user_id, standard_queue_len);
		pthread_mutex_unlock(&enqueue_mutex);
	} else {
		printf("%d's class type is invalid \n", customer->user_id);
	}
	double queue_started;
	double queue_ended;
	double service_ended;

	queue_started = calculate_time(init_time);
	customer->start = queue_started;
	pthread_mutex_lock(&cust_mutex[customer->user_id-1]);
	pthread_cond_wait(&cust_convar[customer->user_id-1], &cust_mutex[customer->user_id-1]);
	pthread_mutex_unlock(&cust_mutex[customer->user_id-1]);
	queue_ended = calculate_time(init_time);
	customer->end = queue_ended;

	int clerk_num;							// This needed to remember which clerk a customer had
	for (int i=0; i<NUM_CLERK; i++){
		if (cust_to_clerk[i] == customer->user_id){
			clerk_num = i;
			break;
		}
	}
	printf("A clerk starts serving a customer: start time %lf, the customer ID %2d, the clerk ID %1d. \n", queue_ended, customer->user_id, clerk_num);
	usleep(customer->service_time);
	printf("A clerk finishes serving a customer: end time %lf, the customer ID %2d, the clerk ID %1d. \n", service_ended, customer->user_id, clerk_num);
	pthread_cond_broadcast(&clerk_convar[clerk_num]);
	cust_to_clerk[clerk_num] = 0;

	double wait_time = customer->end - customer->start;		// For calculating wait times at the end of program
	if (customer->class_type == 0){
		//pthread_mutex_lock(&standard_time);
		standard_wait += (wait_time);
		//pthread_mutex_unlock(&standard_time);
	}else if (customer->class_type == 1){
		//pthread_mutex_lock(&business_time);
		business_wait += (wait_time);
		//pthread_mutex_unlock(&business_time);
	}
	total_wait += (wait_time);
}


int main(int argc, char const *argv[]) { // Main function. This initializes each mutex, customer and clerk thread. It also reads the file into data structure.

	// initialize all the condition variable and thread lock will be used
	
	/** Read customer information from txt file and store them in the structure you created 
		1. Allocate memory(Queue) to store the customer information.
		2. File operation: fopen
	*/
	if(argc<=1){
		printf("input not valid \n");
		return(-1);
	}
	FILE *f = fopen(argv[1], "r");
	if (f == NULL){
		printf("Error when trying to read from file \n");
		return 0;
	}
	char buff[256];
	fgets(buff, 256, f);
	num_cust = atoi(buff);
	gettimeofday(&init_time, NULL);
	business_queue = createQueue(num_cust);
	standard_queue = createQueue(num_cust);


	int user_id;
	int arr_time;
	int ser_time;
	int class_type;

	
	for(int i=0; i<num_cust; i++){
		fscanf(f, "%d:%d,%d,%d", &user_id, &class_type, &arr_time, &ser_time);
		file_storage[i].user_id = user_id;
		file_storage[i].class_type = class_type;
		file_storage[i].arrival_time = arr_time;
		file_storage[i].service_time=ser_time;

	}
	fclose(f);

	for(int i=0; i<MAX_CUST; i++){
		pthread_mutex_init(&cust_mutex[i], NULL);
		pthread_mutex_init(&cust_convar[i], NULL);
	}

	for(int i=0; i<NUM_CLERK; i++){
		pthread_mutex_init(&clerk_mutex[i], NULL);
		pthread_mutex_init(&clerk_convar[i], NULL);
	}

	if (pthread_mutex_init(&qbus_mutex, NULL)!=0){
		printf("business queue init error \n");
		exit(0);
	}

	if (pthread_mutex_init(&qstan_mutex, NULL)!=0){
		printf("standard queue init error \n");
		exit(0);
	}
	if (pthread_mutex_init(&enqueue_mutex, NULL)!=0){
		printf("enqueue queue init error \n");
		exit(0);
	}

	if (pthread_mutex_init(&served_customers, NULL)!=0){
		printf("served customers init error \n");
		exit(0);
	}

	if (pthread_mutex_init(&business_time, NULL)!=0){
		printf("business queue timer init error \n");
		exit(0);
	}
	if (pthread_mutex_init(&standard_time, NULL)!=0){
		printf("standard queue timer init error \n");
		exit(0);
	}
		if (pthread_mutex_init(&build_cust, NULL)!=0){
		printf("standard queue timer init error \n");
		exit(0);
	}
	if (pthread_mutex_init(&build_clerk, NULL)!=0){
		printf("standard queue timer init error \n");
		exit(0);
	}



	pthread_t cust_thread[num_cust];
	pthread_t clerk_thread[NUM_CLERK];
	//struct clerk_data clerk_struct[NUM_CLERK];


	//Create passenger threads
	for (int i=0; i<num_cust;i++){
		pthread_mutex_lock(&build_cust);
		if(pthread_create(&cust_thread[i], NULL, customer_entry, &file_storage[i])!=0){
			printf("error creating passenger thread with user_id %d.\n", file_storage[i].user_id);
			//exit(0);
		}
		pthread_mutex_unlock(&build_cust);
	}
	usleep(10);



	//Create clerk threads
	for (int i=0; i<NUM_CLERK;i++){
		
		int* clerk_id = malloc(sizeof(int));
		*clerk_id = i;

		if(pthread_create(&clerk_thread[i], NULL, clerk_entry, clerk_id)!=0){
			printf("error creating clerk thread with i of %d.\n",i);
			exit(0);
		}

		free(clerk_id);
		
	}



/*		// Join threads
	for (int i=0; i<num_cust; i++){
		if(pthread_join(cust_thread[i], NULL)!=0){
			printf("error joining customer thread with i of %d.\n",i);
			exit(0);
		}
	}

	for (int i=0; i<NUM_CLERK; i++){
		if(pthread_join(cust_thread[i], NULL)!=0){
			printf("error joining cerk thread with i of %d.\n",i);
			exit(0);
		}
	}*/



	sleep(12);
	double avg_total_wait = (total_wait)/num_cust;
	double avg_business_wait = (business_wait/business_queue_max);
	double avg_standard_wait = (standard_wait/standard_queue_max);

	printf("The average waiting time for all customers in the system is: %lf seconds. \n", avg_total_wait);
	printf("The average waiting time for all business-class customers is: %lf seconds. \n",avg_business_wait);
	printf("The average waiting time for all economy-class customers is: %lf seconds. \n", avg_standard_wait);
	
	// Destroy mutexes
	for(int i=0; i<MAX_CUST; i++){
		pthread_mutex_destroy(&cust_mutex[i]);
		pthread_mutex_destroy(&cust_convar[i]);
	}

	for(int i=0; i<NUM_CLERK; i++){
		pthread_mutex_destroy(&clerk_mutex[i]);
		pthread_mutex_destroy(&clerk_convar[i]);
	}
	pthread_mutex_destroy(&qbus_mutex);
	pthread_mutex_destroy(&qstan_mutex);
	pthread_mutex_destroy(&served_customers);
	pthread_mutex_destroy(&business_time);
	pthread_mutex_destroy(&standard_time);

	return 0;
}

