#include <pthread.h>

typedef struct requester{
    queue* buff_ptr;
    FILE* srv_file;
		int start;
		int *serviced;
		int num_files;
		pthread_mutex_t* buff_mutex;
    pthread_mutex_t* srv_mutex;
		pthread_mutex_t* sf_mutex;
		FILE* infiles[10];
}requester;

typedef struct resolver{
    queue* buff_ptr;
    FILE* res_file;
		int * req_remaining;
		pthread_mutex_t* buff_mutex;
    pthread_mutex_t* res_mutex;
}resolver;
