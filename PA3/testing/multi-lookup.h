#include <pthread.h>
//functions which the threads will run
void* request(void*);
void* resolve(void*);
//structs to store info to pass to threads
typedef struct requester{
    queue* buff_ptr;
    FILE* srv_file;
		int start;
		int *serviced;
		int num_files;
		pthread_mutex_t* buff_mutex;
    pthread_mutex_t* srv_mutex;
		pthread_mutex_t* sf_mutex;
    pthread_cond_t* empty_q;
    pthread_cond_t* full_q;
		FILE* infiles[10];
}requester;

typedef struct resolver{
    queue* buff_ptr;
    FILE* res_file;
		int * req_remaining;
		pthread_mutex_t* buff_mutex;
    pthread_mutex_t* res_mutex;
    pthread_cond_t* empty_q;
    pthread_cond_t* full_q;
}resolver;
