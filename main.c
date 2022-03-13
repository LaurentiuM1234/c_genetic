#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "error.h"
#include "worker.h"
#include "task.h"


int main(int argc, char *argv[])
{
        struct worker *workers;

        int ret = create_workers(&workers, argc, argv);

        if (ret < 0) {
                printerr("main", ret);
                exit(EXIT_FAILURE);
        }

        int worker_cnt = workers[0]._shared->_worker_cnt;
        pthread_t handles[worker_cnt];

        ret = start_workers(workers, worker_cnt, handles, task);

        if (ret < 0) {
                free_workers(workers);
                printerr("main", ret);
                exit(EXIT_FAILURE);
        }

        ret = join_workers(worker_cnt, handles);

        if (ret < 0) {
                free_workers(workers);
                printerr("main", ret);
                exit(EXIT_FAILURE);
        }
        
        free_workers(workers);

        return 0;
}