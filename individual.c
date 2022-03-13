#include <stdlib.h>

#include "individual.h"

void free_population(struct individual *population, int size)
{
        for (int i = 0; i < size; i++) {
                if (population[i]._chromosome != NULL)
                        free(population[i]._chromosome);
        }
        free(population);
}