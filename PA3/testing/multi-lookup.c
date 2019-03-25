/*
multi-lookup.c
Sofia Lange
CSCI3753 Programming Assignment 3
DNS name resolution engine
*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "util.h"
#include "queue.h"
#include "multi-lookup.h"



void* request(void* info){
	struct requester* req = info;
  char hostname[1024]; //hostname char arrays
	int fin = 0;
	FILE* infile = req->infiles[req->start]; //Input file
	FILE* srv_file = req->srv_file;
	pthread_mutex_t* buff_mutex = req->buff_mutex; //Buffer mutex
	pthread_mutex_t* srv_mutex = req->srv_mutex;
	pthread_mutex_t* sf_mutex = req->sf_mutex;
	pthread_cond_t* empty_q = req->empty_q;
	pthread_cond_t* full_q = req->full_q;
	queue* buffer = req->buff_ptr; //Queue
	int num_files = req->num_files;
	int num_serviced = *req->serviced;
	int my_serviced = 1;
	while(num_serviced<=num_files){
   	while(fscanf(infile,"%1024s", hostname) > 0){ //while there are still lines in the current file, read and push onto bufferr
			while(!fin){ //we might have to keep looping if the queue is locked when we try to access
				pthread_mutex_lock(buff_mutex);
				while(queue_is_full(buffer)){ //When queue is full
					pthread_cond_wait(full_q, buff_mutex);
				}
        	queue_push(buffer, &hostname);
			pthread_mutex_unlock(buff_mutex);

			pthread_cond_signal(empty_q);
			fin = 1; //Indicate that this hostname was pushed successfully
    	}
		fin = 0; //Reset pushed for the next hostname
		}
		//if we make it to the end, check if there are still files left and service them
		pthread_mutex_lock(sf_mutex);
		*req->serviced += 1;
		pthread_mutex_unlock(sf_mutex);
		num_serviced = *req->serviced;
		if(num_serviced == num_files)
			break;
		infile = req->infiles[num_serviced];
		my_serviced +=1;
	}
	//now we can output how many files we read to serviced.txt
	pthread_mutex_lock(srv_mutex); //lock output file, if possible
	fprintf(srv_file, "Thread %u serviced %d files.\n", (unsigned int)pthread_self(), my_serviced ); //write to output file
	pthread_mutex_unlock(srv_mutex); //unlock output, if possible
	printf("Thread %u serviced %d files.\n", (unsigned int)pthread_self(), 1 ); //write to output file
    return NULL;
}

void* resolve(void* info){
	struct resolver* rslv = info;
  char* hostname;
	FILE* res_file = rslv->res_file;
	pthread_mutex_t* buff_mutex = rslv->buff_mutex;
	pthread_mutex_t* res_mutex = rslv->res_mutex;
	pthread_cond_t* empty_q = rslv->empty_q;
	pthread_cond_t* full_q = rslv->full_q;
	queue* buffer = rslv->buff_ptr;
	char ipstr[INET6_ADDRSTRLEN];
  while(*rslv->req_remaining){ //while producers are still going, keep trying to pop from queue
			//lock and pop off buffer
			pthread_mutex_lock(buff_mutex);
			while((hostname = queue_pop(buffer)) == NULL){
				if(!*rslv->req_remaining){
					pthread_mutex_unlock(buff_mutex);
					return NULL;
				}
				pthread_cond_wait(empty_q, buff_mutex);
			}
			pthread_mutex_unlock(buff_mutex);
			pthread_cond_signal(full_q);
			if(dnslookup(hostname, ipstr, sizeof(ipstr)) == UTIL_FAILURE)
					strncpy(ipstr, "", sizeof(ipstr));

			//write to the output file
      pthread_mutex_lock(res_mutex);
			fprintf(res_file, "%s,%s\n", hostname, ipstr); //write to output file
			pthread_mutex_unlock(res_mutex); //unlock output, if possible
    }
			return NULL;
	}

int main(int argc, char * argv[]){
	/*
	argv[1] number of requesters
	argv[2] number of resolvers
	argv[3] request log file
	argv[4] resolve log file
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
	pthread_cond_t empty_q;
	pthread_cond_t full_q;
// arrays of requester/resolver information structs to pass to the threads
	struct requester req_threads[num_requesters];
	struct resolver res_threads[num_resolvers];
// now that we are done declaring, initialize queue and buffer
	queue_init(&buffer, 10);

	pthread_mutex_init(&buff_mutex, NULL);
	pthread_mutex_init(&srv_mutex, NULL);
	pthread_mutex_init(&res_mutex, NULL);
	pthread_mutex_init(&sf_mutex, NULL);

	int m = pthread_cond_init(&empty_q, NULL);
	int n = pthread_cond_init(&full_q, NULL);
	printf("%d , %d\n", m,n);
// open all files and check for errors
	srv_file = fopen(argv[3], "w");
	if(!srv_file){
		fprintf(stderr,"Bogus Output File Name : Request Log\n");
		return -1;
	}
	res_file = fopen(argv[4], "w");
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
			req_threads[i].empty_q = &empty_q; //condition vars to indicate full and empty queue
			req_threads[i].full_q = &full_q;

    	pthread_create(&(requests[i]), NULL, request, &(req_threads[i]));
    }
//the resolvers
    for(int i=0; i<num_resolvers; i++){
				res_threads[i].res_file = res_file; // results.txt
				res_threads[i].res_mutex = &res_mutex; // mutex for results.txt
				res_threads[i].buff_mutex = &buff_mutex; // shared buffer mutex
				res_threads[i].buff_ptr = &buffer; // actual shared buffer
				res_threads[i].req_remaining = &req_remaining; // int to indicate if there are still producers working
				req_threads[i].empty_q = &empty_q; //condition vars to indicate full and empty queue
				req_threads[i].full_q = &full_q;

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

	pthread_cond_destroy(&empty_q);
	pthread_cond_destroy(&full_q);
	gettimeofday(&t1,0);
	long elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;
	printf("Total Runtime : %ld\n",elapsed);
    return 0;
}
