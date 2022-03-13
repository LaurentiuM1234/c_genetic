#ifndef _SACK_H
#define _SACK_H

struct sack_object {
        int _weight;
        int _profit;
};

struct sack {
        int _capacity;
        int _object_count;
        struct sack_object *_objects;
};

/*
 * Description: Allocate memory and set fields of a sack object.
 * 
 * Parameters: - sack = address of pointer to sack object to be allocated.
 *             - capacity = number representing the sack's capacity.
 *             - object_count = amount of objects in sack.
 *             - objects = array of objects found in the sack.
 * 
 * Returns: -ELIB = Failed library call.
 */
int create_sack(struct sack **sack, int capacity,
                int object_count, struct sack_object *objects);

/*
 * Description: Free memory occupied by sack object.
 * 
 * Parameters: - sack = pointer to object to be freed.
 */
void free_sack(struct sack *sack);
#endif // _SACK_H