#ifndef _TASK_H
#define _TASK_H

/*
 * Description: Task to be executed by a worker.
 * 
 * Parameters: - info = pointer to data received by worker.
 * 
 * Returns: NULL
 */
void *task(void *info);

#endif // _TASK_H