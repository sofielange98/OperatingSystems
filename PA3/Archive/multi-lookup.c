#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "util.h"
#include "queue.h"
#include "multi-lookup.h"

void* request(void* info){
	struct requester* thread = info; //Make a thread to hold info
  char hostname[1024]; //hostname char arrays
	int fin = 0;
	FILE* inputfp = thread->infiles[thread->start]; //Input file
	FILE* srv_file = thread->srv_file;
	pthread_mutex_t* buff_mutex = thread->buff_mutex; //Buffer mutex
	pthread_mutex_t* srv_mutex = thread->srv_mutex;
	pthread_mutex_t* sf_mutex = thread->sf_mutex;
	queue* buffer = thread->buff_ptr; //Queue
	int num_files = thread->num_files;
	int num_serviced = *thread->serviced;
	int my_serviced = 0;

	//printf("File number %d has been assigned to thread %u\n.", thread->start, (unsigned int)pthread_self());
	while(num_serviced<=num_files){
   	while(fscanf(inputfp,"%1024s", hostname) > 0){ //Read input and push onto buffer
			while(!fin){ //Repeat until we were able to access and push the hostname
				pthread_mutex_lock(buff_mutex);
				while(queue_is_full(buffer)){
					pthread_mutex_unlock(buff_mutex);
					usleep((rand() % 100)+13);
					pthread_mutex_lock(buff_mutex);
			}
        	queue_push(buffer, &hostname);
			pthread_mutex_unlock(buff_mutex);
			fin = 1; //we did it!
    	}
		fin = 0; //Reset for next hostname
		}
		my_serviced += 1; // we read one file
		//printf("Reached the end of one file in thread number %u\n", (unsigned int)pthread_self());
		pthread_mutex_lock(sf_mutex);
		*thread->serviced += 1;
		pthread_mutex_unlock(sf_mutex);
		num_serviced = *thread->serviced;
		if(num_serviced > num_files){
			break;
		}
		//printf("Thread number %u being reassigned to new file number %d.\n", (unsigned int)pthread_self(), num_serviced);
		inputfp = thread->infiles[num_serviced-1];
	}
	//now we can output how many files we read to serviced.txt
	pthread_mutex_lock(srv_mutex);
	fprintf(srv_file, "Thread %u serviced %d files.\n", (unsigned int)pthread_self(), my_serviced ); //write to output file
	pthread_mutex_unlock(srv_mutex);
	//printf("Thread %u serviced %d files.\n", (unsigned int)pthread_self(), 1 );
    return NULL;
}

void* resolve(void* info){
	struct resolver* thread = info; //Make a thread to hold info
   	char* domain; //domain char arrays
	FILE* res_file = thread->res_file; //Output file
	pthread_mutex_t* buff_mutex = thread->buff_mutex; //Buffer mutex
	pthread_mutex_t* res_mutex = thread->res_mutex; //Output mutex
	queue* buffer = thread->buff_ptr; //Queue
	char ipstr[INET6_ADDRSTRLEN]; //IP Addresses
    while(!queue_is_empty(buffer) || *thread->req_remaining){ //while the queue has stuff or there's request threads, loop
		pthread_mutex_lock(buff_mutex); //lock buffer
		domain = queue_pop(buffer); //pop off queue
		if(domain == NULL){ //if empty, unlock
			pthread_mutex_unlock(buff_mutex);
			usleep((rand() % 100)+13);
		}
		else { //Unlock and go!
			pthread_mutex_unlock(buff_mutex);
			if(dnslookup(domain, ipstr, sizeof(ipstr)) == UTIL_FAILURE)//look up domain, or try
				strncpy(ipstr, "", sizeof(ipstr));
			//printf("%s:%s\n", domain, ipstr);
      pthread_mutex_lock(res_mutex); //lock output file, if possible
			fprintf(res_file, "%s,%s\n", domain, ipstr); //write to output file
			pthread_mutex_unlock(res_mutex); //unlock output, if possible
    	}
			//free(domain);
	}
    return NULL;
}

int main(int argc, char * argv[]){
	/*
	argv[1] number of requesters
	argv[2] number of resolvers
	argv[3] resolve log file / results.txt
	argv[4] request log file / serviced.txt
	argv[5:argc-1] name files
	*/
// start timer
	struct timeval t0;
	struct timeval t1;
	gettimeofday(&t0, 0);
//make sure they follow the rules
	if(argc-5>10){
		printf("No sir, only 10 input files allowed, try again.\n");
		return -1;
	}
	if(atoi(argv[2])>10){
		printf("No sir, can't do more than 10 resolvers, try again.\n");
	}
	if(atoi(argv[1])>5){
		printf("No sir, can't do more than 5 requesters, try again.\n");
	}
// initialize all the variables
	queue buffer;
	int num_infiles = argc-5;
	FILE* res_file = NULL;
	FILE* srv_file = NULL;
	FILE* inputs[num_infiles];
	int num_resolvers = atoi(argv[2]);
	int num_requesters = atoi(argv[1]);
	int serviced = num_requesters;
	int req_remaining = 1;
// arrays of threads
	pthread_t requests[num_requesters];
	pthread_t resolves[num_resolvers];
// all the mutexes
	pthread_mutex_t buff_mutex;
	pthread_mutex_t srv_mutex;
	pthread_mutex_t res_mutex;
	pthread_mutex_t sf_mutex;
// conditions for signalling
	//pthread_cond_t empty_q;
	//pthread_cond_t full_q;
// arrays of requester/resolver information structs to pass to the threads
	struct requester req_threads[num_requesters];
	struct resolver res_threads[num_resolvers];
// now that we are done declaring, initialize queue and buffer
	queue_init(&buffer, 10);

	pthread_mutex_init(&buff_mutex, NULL);
	pthread_mutex_init(&srv_mutex, NULL);
	pthread_mutex_init(&res_mutex, NULL);
	pthread_mutex_init(&sf_mutex, NULL);

//	int m = pthread_cond_init(&empty_q, NULL);
//	int n = pthread_cond_init(&full_q, NULL);
//	printf("%d , %d\n", m,n);
// open all files and check for errors
	srv_file = fopen(argv[4], "w");
	if(!srv_file){
		fprintf(stderr,"Bogus Output File Name : Request Log\n");
		return -1;
	}
	res_file = fopen(argv[3], "w");
	if(!res_file){
		fprintf(stderr,"Bogus Output File Name : Resolve Log\n");
		return -1;
	}
	for(int i=0; i<num_infiles; i++){
		inputs[i] = fopen(argv[i+5], "r");
		if(!inputs[i]){
			fprintf(stderr, "Bogus Input File Name : Name File %d\n", i);
		}
	}

// Now the moment we've all been waiting for
// the requesters
	for(int i=0; i<num_requesters; i++){
		//set alllllll the data then create the thread
			for(int j = 0; j<num_infiles; j++){ //pass array of input file pointers to each thread, so they can change if they finish
				req_threads[i].infiles[j] = inputs[j];
			}
			req_threads[i].buff_mutex   = &buff_mutex; // mutex for the shared buffer of hostnames
			req_threads[i].srv_mutex    = &srv_mutex; // mutex for service file
			req_threads[i].sf_mutex = &sf_mutex; // mutex for int number of files that have been serviced by any requester
			req_threads[i].srv_file = srv_file; //serviced.txt
			req_threads[i].buff_ptr = &buffer; // the actual buffer
			req_threads[i].start = i; // where this thread should start in the infiles array
			req_threads[i].num_files = num_infiles; // how many total files need to be serviced
			req_threads[i].serviced = &serviced; // how many files have been serviced, shared int
			//req_threads[i].empty_q = &empty_q; //condition vars to indicate full and empty queue
			//req_threads[i].full_q = &full_q;

    	pthread_create(&(requests[i]), NULL, request, &(req_threads[i]));
    }
//the resolvers
    for(int i=0; i<num_resolvers; i++){
				res_threads[i].res_file = res_file; // results.txt
				res_threads[i].res_mutex = &res_mutex; // mutex for results.txt
				res_threads[i].buff_mutex = &buff_mutex; // shared buffer mutex
				res_threads[i].buff_ptr = &buffer; // actual shared buffer
				res_threads[i].req_remaining = &req_remaining; // int to indicate if there are still producers working
				//req_threads[i].empty_q = &empty_q; //condition vars to indicate full and empty queue
				//req_threads[i].full_q = &full_q;

				pthread_create(&(resolves[i]), NULL, resolve, &(res_threads[i]));
    }

  for(int i=0; i<num_requesters; i++){ // wait for all the requester threads to finish
      pthread_join(requests[i], NULL);
	}
// now all requesters have finished reading files, so once resolvers are done they can exit
	req_remaining=0;

  for(int i=0; i<num_resolvers; i++){ // wait for resolvers to finish
  		pthread_join(resolves[i], NULL);
	}

	queue_cleanup(&buffer); // bye bye queue
	// close all the files
	fclose(res_file);
	fclose(srv_file);
	for(int i=0; i<num_infiles; i++){
		fclose(inputs[i]);
	}
	// kill all the mutexes
	pthread_mutex_destroy(&buff_mutex);
	pthread_mutex_destroy(&res_mutex);
	pthread_mutex_destroy(&srv_mutex);
	pthread_mutex_destroy(&sf_mutex);

	gettimeofday(&t1,0);
	long elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;
	printf("Total Runtime : %ld\n",elapsed);
    return 0;
}
