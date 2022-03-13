#include <stdlib.h>

#include "sack.h"
#include "error.h"


int create_sack(struct sack **sack, int capacity,
                int object_count, struct sack_object *objects)
{
        *sack = calloc(1, sizeof(struct sack));

        if (*sack == NULL)
                return -ELIB;

        (*sack)->_capacity = capacity;
        (*sack)->_object_count = object_count;
        (*sack)->_objects = objects;

        return 0;
}

void free_sack(struct sack *sack)
{
        free(sack->_objects);
        free(sack);
}