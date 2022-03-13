#ifndef _INDIVIDUAL_H
#define _INDIVIDUAL_H

struct individual {
        int _fitness;
        int *_chromosome;
        int _chromosome_len;
        int _index;
};

/*
 * Description: Free memory occupied by an array of individuals.
 * 
 * Parameters: - population = array to be freed.
 *             - size = number of individuals in array.
 */
void free_population(struct individual *population, int size);

#endif // _INDIVIDUAL_H