#ifndef POOL_H
#define POOL_H
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<time.h>
#include<ctype.h>
#include "pool.h"
#include "trie.h"

#define MAX 10000 /* Max length of strings used here */
#define MAX_WAIT_TIME_IN_SECONDS 4 /* Idle time for thread to
                                      wait and then exit */
//thread_pool structure defintion

struct thread_pool
{
  pthread_mutex_t mtx; /* Mutex to access an instance of this stucture */
  pthread_cond_t cnd; /* Condition variable to wait on
                         to get a filename on the queue */
  pthread_t *threads; /* List of threads in this pool */
  char **files; /* Queue of strings to hold filenames to read by threads */

  int thread_count; /* Number of threads in this pool */
  int file_count; /* Max number of files allowed on the
                     Queue (max queue size) */
  int head; /* Head of filename queue */
  int tail; /* Tail of filename queue */
  int count; /* Number of items in the queue */
};

/* Function that allocates resources and
 * initializes an instance of the thread_pool
 * @param: num_threads: Number of threads in pool
 * @param: num_files: Max number of files in queue
 * @return: pointer to an instance of initialized thread_pool struct
 */
struct thread_pool *create_thread_pool(int num_threads, int num_files);

/* Function that frees resources of thread_pool
 * object pointed to by given pointer
 * @param: pointer to thread_pool object
 */
int free_thread_pool(struct thread_pool *pool);

/* Function to add file to filenames queue for processing by threads
 * @param: pool: pointer to thread_pool object
 * @param: filename: string filename to add to queue
 */
void add_file(struct thread_pool *pool, char *filename);
#endif
