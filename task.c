#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "task.h"
#include "individual.h"
#include "worker.h"

static int cmpfunc(const void *a, const void *b)
{
	int i;
	struct individual *first = (struct individual *) a;
	struct individual *second = (struct individual *) b;

	int res = second->_fitness - first->_fitness;
	if (res == 0) {
		int first_count = 0, second_count = 0;

		for (i = 0; i < first->_chromosome_len &&
                                i < second->_chromosome_len; ++i) {
			first_count += first->_chromosome[i];
			second_count += second->_chromosome[i];
		}

		res = first_count - second_count; 
		if (res == 0) {
			return second->_index - first->_index;
		}
	}
	return res;
}
static void copy_chromosome(struct individual *from, struct individual *to)
{
	memcpy(to->_chromosome,
                from->_chromosome,
                from->_chromosome_len * sizeof(int));
}

static void copy_individual(struct individual *from, struct individual *to)
{

        to->_fitness = from->_fitness;
        to->_index = from->_index;
        to->_chromosome_len = from->_chromosome_len;

	memcpy(to->_chromosome,
                from->_chromosome,
                from->_chromosome_len * sizeof(int));
}

static int individual_fitness(struct individual crt, struct sack *sack)
{
        int weight = 0, profit = 0;

        for (int j = 0; j < crt._chromosome_len; j++) {
                if (crt._chromosome[j]) {
                        weight += sack->_objects[j]._weight;
                        profit += sack->_objects[j]._profit;
                }
        }
        return (weight <= sack->_capacity) ? profit : 0;
}

static void population_fitness(struct data *data, int worker_id,
                                        int worker_count)
{
        int zone_size = data->_sack->_object_count;

        int start = worker_id * zone_size / worker_count;
        int end = fmin((worker_id + 1) * zone_size / worker_count, zone_size);

        struct individual *crt = data->_crt_generation;

        for (int i = start; i < end; i++) {
                crt[i]._fitness = individual_fitness(crt[i], data->_sack);
        }
}


static void prepare_populations(struct data *data, int worker_id,
                                        int worker_count)
{
        int zone_size = data->_sack->_object_count;

        int start = worker_id * zone_size / worker_count;
        int end = fmin((worker_id + 1) * zone_size / worker_count, zone_size);

        for (int i = start; i < end; i++) {
                data->_crt_generation[i]._fitness = 0;
                data->_crt_generation[i]._chromosome =
                                calloc(zone_size, sizeof(int));
                data->_crt_generation[i]._chromosome[i] = 1;
                data->_crt_generation[i]._index = i;
                data->_crt_generation[i]._chromosome_len = zone_size;

                data->_next_generation[i]._fitness = 0;
                data->_next_generation[i]._chromosome =
                                calloc(zone_size, sizeof(int));
                data->_next_generation[i]._index = i;
                data->_next_generation[i]._chromosome_len = zone_size;

                data->_left_dump[i]._fitness = 0;
                data->_left_dump[i]._chromosome =
                                calloc(zone_size, sizeof(int));
                data->_left_dump[i]._index = i;
                data->_left_dump[i]._chromosome_len = zone_size;

                data->_right_dump[i]._fitness = 0;
                data->_right_dump[i]._chromosome =
                                calloc(zone_size, sizeof(int));
                data->_right_dump[i]._index = i;
                data->_right_dump[i]._chromosome_len = zone_size;
        }
}

static void revert_indexes(struct data *data, int worker_id,
                                        int worker_count)
{
        int zone_size = data->_sack->_object_count;

        int start = worker_id * zone_size / worker_count;
        int end = fmin((worker_id + 1) * zone_size / worker_count, zone_size);

        for (int i = start; i < end; i++) {
                data->_crt_generation[i]._index = i;
        }
}


static void copy_fittest(struct data *data, int worker_id,
                                        int worker_count)
{
        struct individual *crt = data->_crt_generation;
        struct individual *next = data->_next_generation;

        int zone_size = data->_sack->_object_count * 0.3;

        int start = worker_id * zone_size / worker_count;
        int end = fmin((worker_id + 1) * zone_size / worker_count,
                                zone_size);

        for (int i = start; i < end; i++) {
                copy_chromosome(crt + i, next + i);
        }

}

static void mutate_bit_string_1(struct individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->_chromosome_len - 2);

	if (ind->_index % 2 == 0) {
		mutation_size = ind->_chromosome_len * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->_chromosome[i] = 1 - ind->_chromosome[i];
		}
	} else {
		mutation_size = ind->_chromosome_len * 8 / 10;
		for (i = ind->_chromosome_len - mutation_size;
                        i < ind->_chromosome_len; i += step) {
			ind->_chromosome[i] = 1 - ind->_chromosome[i];
		}
	}
}

static void mutate_bit_string_2(struct individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->_chromosome_len - 2);

	for (int i = 0; i < ind->_chromosome_len; i += step) {
		ind->_chromosome[i] = 1 - ind->_chromosome[i];
	}
}

static void mutate_poplation(struct data *data,
                                int worker_id, int worker_count)
{
        struct individual *crt = data->_crt_generation;
        struct individual *next = data->_next_generation;

        int zone_size = data->_sack->_object_count * 0.4;

        int start = worker_id * zone_size / worker_count;
        int end = fmin((worker_id + 1) *
                        zone_size / worker_count, zone_size);

        int threshold = data->_sack->_object_count * 0.2;

        int pos = 0.3 * data->_sack->_object_count;

        for (int i = start; i < end; i++) {
                if (i < threshold) {
                        copy_chromosome(crt + i, next + pos + i);
                        mutate_bit_string_1(next + pos + i, data->_gen_idx);
                } else {
                        copy_chromosome(crt + i, next + pos + i);
                        mutate_bit_string_2(next + pos + i, data->_gen_idx);
                }
        }
}

static void crossover(struct individual *p1,
                        struct individual *c1, int generation_index)
{
	struct individual *p2 = p1 + 1;
	struct individual *c2 = c1 + 1;
	int count = 1 + generation_index % p1->_chromosome_len;

	memcpy(c1->_chromosome, p1->_chromosome, count * sizeof(int));
	memcpy(c1->_chromosome + count, p2->_chromosome + count,
                (p1->_chromosome_len - count) * sizeof(int));

	memcpy(c2->_chromosome, p2->_chromosome, count * sizeof(int));
	memcpy(c2->_chromosome + count, p1->_chromosome + count,
                (p1->_chromosome_len - count) * sizeof(int));
}

void crossover_population(struct data *data, int worker_id, int worker_count)
{
        struct individual *crt = data->_crt_generation;
        struct individual *next = data->_next_generation;

        int object_count = data->_sack->_object_count;
        int zone_size = object_count * 0.3;

        int pos = object_count * 0.7;

        if (zone_size % 2 == 1) {
                if (worker_id == 0) {
                        copy_chromosome(crt + object_count - 1,
                                next + pos + zone_size - 1);
                }
		zone_size--;
        }

        int start = worker_id * zone_size / worker_count;
        int end = fmin((worker_id + 1) *
                                zone_size / worker_count, zone_size);

        if (start % 2 != 0)
                start += 1;

        for (int i = start; i < end; i += 2) {
                crossover(crt + i, next + pos + i, data->_gen_idx);
        }
}

static void merge_partitions(struct data *data, int start, int mid, int stop)
{
        int d1_edge = mid - start;
        int d2_edge = stop - mid;


        for (int i = 0; i < d1_edge; i++) {
                copy_individual(data->_crt_generation + i + start,
                                        data->_left_dump + i);
        }

        for (int i = 0; i < d2_edge; i++) {
                copy_individual(data->_crt_generation + i + mid,
                                        data->_right_dump + i);
        }

        int i = 0, j = 0, k = start;

        while (i < d1_edge && j < d2_edge) {
                if (cmpfunc(data->_left_dump + i, data->_right_dump + j) < 0) {
                        copy_individual(data->_left_dump + i,
                                                data->_crt_generation + k);
                        i++;
                        k++;
                } else {
                        copy_individual(data->_right_dump + j,
                                                data->_crt_generation + k);
                        j++;
                        k++;
                }
        }

        while (i < d1_edge) {
                copy_individual(data->_left_dump + i,
                                        data->_crt_generation + k);
                i++;
                k++;
        }

        while (j < d2_edge) {
                copy_individual(data->_right_dump + j,
                                        data->_crt_generation + k);
                j++;
                k++;
        }
}
static void merge_sort(struct data *data, int worker_id, int worker_count)
{
        if(worker_id > 4) {
                pthread_barrier_wait(&data->_barrier);
                return;
        }

        if (worker_count > 4)
                worker_count = 4;

        int zone_size = data->_sack->_object_count;
        int partition_size = zone_size / worker_count;

        int start = worker_id * partition_size;
        int end = start + partition_size;

        if (worker_id == worker_count - 1)
          end = zone_size;

        qsort(data->_crt_generation + start,
              end - start, sizeof(struct individual), cmpfunc);

        pthread_barrier_wait(&data->_barrier);

        if (worker_id == 0) {
                if (worker_count == 4) {
                        merge_partitions(data, 0,
                                partition_size, 2 * partition_size);
                        merge_partitions(data, 2 * partition_size,
                                3 * partition_size, 4 * partition_size);

                        merge_partitions(data, 0,
                                2 * partition_size, zone_size);
                } else if (worker_count == 3) {
                        merge_partitions(data, 0, partition_size,
                                2 * partition_size);
                        merge_partitions(data, 0, 2 * partition_size,
                                zone_size);
                } else if (worker_count == 2) {
                        merge_partitions(data, 0, zone_size / 2, zone_size);
                }
        }
}

void local_max_fitness(struct data *data, int worker_id, int worker_count)
{
        int zone_size = data->_sack->_object_count;
        int max_fitness = -1;
        
        int start = worker_id * (double) zone_size / worker_count;
        int end = fmin((worker_id + 1) *
                        (double) zone_size / worker_count, zone_size);

        for (int i = start; i < end; i++) {
                int crt_fitness =
                        individual_fitness(data->_crt_generation[i],
                                                data->_sack);
                if (crt_fitness > max_fitness) 
                        max_fitness = crt_fitness;
        }

        data->_local_maximas[worker_id] = max_fitness;
}

void free_populations(struct data *data, int worker_id, int worker_count)
{
        int zone_size = data->_sack->_object_count;
        int start = worker_id * (double) zone_size / worker_count;
        int end = fmin((worker_id + 1) *
                        (double) zone_size / worker_count, zone_size);

        for (int i = start; i < end; i++) {
                free(data->_crt_generation[i]._chromosome);
                free(data->_next_generation[i]._chromosome);
                free(data->_left_dump[i]._chromosome);
                free(data->_right_dump[i]._chromosome);
        }

        pthread_barrier_wait(&data->_barrier);

        if (worker_id == 0) {
                free(data->_crt_generation);
                free(data->_next_generation);
                free(data->_left_dump);
                free(data->_right_dump);
        }
}

void *task(void *info)
{
        struct worker *input = (struct worker*)info;
        struct data *shared = input->_shared;

        int worker_id = input->_id;
        int worker_cnt = shared->_worker_cnt;

        // prepare required poulations for computation
        prepare_populations(shared, worker_id, worker_cnt);

        // wait for all threads to finish preparation
        pthread_barrier_wait(&shared->_barrier);

        while (shared->_gen_idx < shared->_gen_cnt) {
                population_fitness(shared, worker_id, worker_cnt);

                // wait for fitness computation before sorting
                pthread_barrier_wait(&shared->_barrier);

                merge_sort(shared, worker_id, worker_cnt);

                // wait for sorting to finish
                pthread_barrier_wait(&shared->_barrier);

                // copy the 30% fittest individuals
                copy_fittest(shared, worker_id, worker_cnt);

                // mutate 40% of population
                mutate_poplation(shared, worker_id, worker_cnt);

                crossover_population(shared, worker_id, worker_cnt);

                // wait for crossover and mutation before switching
                pthread_barrier_wait(&shared->_barrier);

                if (worker_id == 0) {
                        struct individual *tmp = shared->_crt_generation;
                        shared->_crt_generation = shared->_next_generation;
                        shared->_next_generation = tmp;

                        if (shared->_gen_idx % 5 == 0) {
                                printf("%d\n",
                                        shared->_crt_generation[0]._fitness);
                        }
                        shared->_gen_idx += 1;
                }

                // wait for worker 0 to switch generations
                pthread_barrier_wait(&shared->_barrier);

                if (shared->_gen_idx < shared->_gen_cnt)
                        revert_indexes(shared, worker_id, worker_cnt);
        }
        local_max_fitness(shared, worker_id, worker_cnt);

        pthread_barrier_wait(&shared->_barrier);

        if (worker_id == 0) {
                int max_fitness = 0;

                for (int i = 0; i < worker_cnt; i++) {
                        if (shared->_local_maximas[i] > max_fitness)
                                max_fitness = shared->_local_maximas[i];
                }
               
                printf("%d\n", max_fitness);
        }

        // free resources taken by the populations
        free_populations(shared, worker_id, worker_cnt);

        return NULL;
}