#include <stdio.h>
#include <stdlib.h>

#include "worker.h"
#include "error.h"

int read_input(struct sack_object **objects, int *object_count,
                int *sack_capacity, int *generations_count,
                int *worker_cnt, int argc, char *argv[])
{
        if (argc != 4)
		return -EBADARG;

        int ret;

	FILE *f_ptr = fopen(argv[1], "r");

	if (f_ptr == NULL)
		return -ELIB;

        ret = fscanf(f_ptr, "%d %d", object_count, sack_capacity);

	if (ret < 2) {
		ret = -EREAD;
		goto err_release_fp;
	}

	if (*object_count % 10) {
		ret = -EBADINPUT;
		goto err_release_fp;
	}

	struct sack_object *tmp_objects = 
                calloc(*object_count, sizeof(struct sack_object));

	for (int i = 0; i < *object_count; ++i) {
                ret = fscanf(f_ptr, "%d %d",
                        &tmp_objects[i]._profit,&tmp_objects[i]._weight);

		if (ret < 2) {
			ret = -EREAD;
                        goto err_free_objects;
		}
	}

	*generations_count = (int) strtol(argv[2], NULL, 10);
	
	if (*generations_count == 0) {
		ret = -EBADINPUT;
		goto err_free_objects;
	}

        *worker_cnt = (int) strtol(argv[3], NULL, 10);
	
	if (*worker_cnt == 0) {
		ret = -EBADINPUT;
		goto err_free_objects;
	}

	*objects = tmp_objects;

        fclose(f_ptr);
	return 0;

err_free_objects:
        free(tmp_objects);
err_release_fp:
        fclose(f_ptr);
        return ret;
}

static int create_data(struct data **data, int argc, char **argv)
{
        int ret;

        struct sack_object *objects;

        int object_count, sack_capacity, generations_count, worker_cnt;

        ret = read_input(&objects, &object_count, &sack_capacity,
                                &generations_count, &worker_cnt, argc, argv);
        
        if (ret < 0)
                return ret;
        
        struct sack *sack;

        ret = create_sack(&sack, sack_capacity, object_count, objects);

        if (ret < 0)
                goto err_free_objects;

        
        *data = calloc(1, sizeof(struct data));

        if (*data == NULL) {
                ret = -ELIB;
                goto err_free_sack;
        }

        (*data)->_worker_cnt = worker_cnt;
        (*data)->_gen_cnt = generations_count;
        (*data)->_sack = sack;

        ret = pthread_barrier_init(&(*data)->_barrier, NULL, worker_cnt);

        if (ret < 0) {
                ret = -ELIB;
                goto err_free_data;
        }

        (*data)->_crt_generation = calloc(object_count,
                                        sizeof(struct individual));

        if ((*data)->_crt_generation == NULL) {
                ret = -ELIB;
                goto err_release_barrier;
        }

        (*data)->_next_generation = calloc(object_count,
                                        sizeof(struct individual));

        if ((*data)->_next_generation == NULL) {
                ret = -ELIB;
                goto err_free_crt_generation;
        }

        (*data)->_left_dump = calloc(object_count,
                                        sizeof(struct individual));

        if ((*data)->_left_dump == NULL) {
                ret = -ELIB;
                goto err_free_next_generation;
        }
        (*data)->_right_dump = calloc(object_count,
                                        sizeof(struct individual));

         if ((*data)->_right_dump == NULL) {
                ret = -ELIB;
                goto err_free_left_dump;
        }

        (*data)->_local_maximas = calloc(worker_cnt, sizeof(int));

        if ((*data)->_local_maximas == NULL) {
                ret = -ELIB;
                goto err_free_right_dump;
        }

        return 0;

err_free_right_dump:
        free((*data)->_right_dump);
err_free_left_dump:
        free((*data)->_left_dump);
err_free_next_generation:
        free((*data)->_next_generation);
err_free_crt_generation:
        free((*data)->_crt_generation);
err_release_barrier:
        pthread_barrier_destroy(&(*data)->_barrier);
err_free_data:
        free(*data);
err_free_sack:
        free_sack(sack);
err_free_objects:
        free(objects);
        return ret;
}

static void free_data(struct data *data)
{
        pthread_barrier_destroy(&data->_barrier);
        free_sack(data->_sack);
        free(data->_local_maximas);
        free(data);
}


int create_workers(struct worker **workers, int argc, char **argv)
{
        struct data *shared;

        int ret = create_data(&shared, argc, argv);

        if (ret < 0)
                return ret;

        int worker_cnt = shared->_worker_cnt;

        *workers = calloc(worker_cnt, sizeof(struct worker));

        if (*workers == NULL) {
                free_data(shared);
                return -ELIB;
        }

        for (int i = 0; i < worker_cnt; i++) {
                (*workers)[i]._id = i;
                (*workers)[i]._shared = shared;
        }

        return 0;
}



void free_workers(struct worker *workers)
{
        free_data(workers[0]._shared);
        free(workers);
}

int start_workers(struct worker *workers, int worker_cnt,
                        pthread_t *handles, void*(*task)(void*))
{
        int ret;

        for (int i = 0; i < worker_cnt; i++) {
                ret = pthread_create(&handles[i], NULL, task, workers + i);

                if (ret < 0)
                        return -ELIB;
        }

        return 0;
}
int join_workers(int worker_cnt, pthread_t *handles)
{
        int ret;

        for (int i = 0; i < worker_cnt; i++) {
                ret = pthread_join(handles[i], NULL);

                if (ret < 0)
                        return -ELIB;
        }

        return 0;
}