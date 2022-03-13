#ifndef _WORKER_H
#define _WORKER_H

#include <pthread.h>

#include "individual.h"
#include "sack.h"

struct data {
        struct individual *_crt_generation;
        struct individual *_next_generation;
        struct sack *_sack;
        int _gen_cnt;
        int _gen_idx;
        pthread_barrier_t _barrier;
        int _worker_cnt;
        int *_local_maximas;
        struct individual *_left_dump;
        struct individual *_right_dump;
};

struct worker {
        int _id;
        struct data *_shared;
};

/*
* Description: Allocate an array of worker objects.
*
* Parameters: - workers = addres of pointer to array to be allocated.
*             - argc = number of arguments received by main function.
*             - argv = argument vector of main function.
*
* Returns: -ELIB = Failed library call.
*          -EBADINPUT = Invalid input.
*          -EREAD = Failed to read data from file.
*          -EBADARG = Bad number of arguments.
*/
int create_workers(struct worker **workers, int argc, char **argv);

/*
* Description: Free memory occupied by workers array.
*
* Parameters: - workers = pointer to array to be freed.
*/
void free_workers(struct worker *workers);

/*
 * Description: Run workers(threads).
 * 
 * Parameters: - workers = pointer to array of worker objects.
 *             - worker_cnt = number of workers to be ran.
 *             - handles = vector of numbers used to refer the each worker.
 *             - task = task to be executed by workers.
 * 
 * Returns: -ELIB = Failed library call.
 */
int start_workers(struct worker *workers, int worker_cnt,
                        pthread_t *handles, void*(*task)(void*));

/**
 * Description: Wait for each worker(thread) to finish its task.
 * 
 * Parameters: - worker_cnt = number of workers to be waited.
 *             - handles = vector of numbers used to refer to each worker.
 * 
 * Returns: -ELIB = Failed library call.
 **/
int join_workers(int worker_cnt, pthread_t *handles);


#endif // _WORKER_H