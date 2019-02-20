#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<time.h>
#include<ctype.h>
#include "trie.h"

#define MAX 10000 /* Max length of strings used here */
#define MAX_WAIT_TIME_IN_SECONDS 4 /* Idle time for thread
                                      to wait and then exit */

//Referring to global Trie
extern struct Trie *trie;

//thread_pool structure defintion
struct thread_pool
{
  pthread_mutex_t mtx; /* Mutex to access an instance of this stucture */
  pthread_cond_t cnd; /* Condition variable to wait
                         on to get a filename on the queue */
  pthread_t *threads; /* List of threads in this pool */
  char **files; /* Queue of strings to hold filenames to read by threads */

  int thread_count; /* Number of threads in this pool */
  int file_count; /* Max number of files allowed
                     on the Queue (max queue size) */
  int head; /* Head of filename queue */
  int tail; /* Tail of filename queue */
  int count; /* Number of items in the queue */
};


/*Funtion that reads given file, finds words and adds them to global trie
 * @param: filename: name of file to be read: string
 */
void read_file(char* filename)
{
  FILE *file = fopen(filename, "r");
  char c;
  int k;
  char word[MAX];

  while(1)
  {
    c = (char)fgetc(file);
    k = 0;
    while(isalnum(c) && !feof(file))
    {
      word[k++] = c;
      c = (char)fgetc(file);
    }
    word[k]=0;
    if(feof(file))
    {
      break;
    }
    //printf("\n%s", word);
    if(word[0] != 0)
      add_to_trie(trie, word);
  }

}

/* Funtion that runs thread - keeps looking for filename in queue to read,
 *                            if wait for a filename timesout, thread exits
 *@param: p: pointer to thread pool
 */
void *thread_run(void *p)
{
  struct thread_pool *pool = (struct thread_pool *)p;
  char *filename = malloc(MAX*sizeof(char));
  int i;

  //Creating abstime until to wait for
  struct timespec time_to_wait = {0, 0};
  time_to_wait.tv_sec = time(NULL) + MAX_WAIT_TIME_IN_SECONDS;
  int ret;
  for(;;)
  {

    pthread_mutex_lock(&(pool->mtx));
    while(pool->count == 0)
      if((ret = pthread_cond_timedwait(&(pool->cnd),&(pool->mtx),
              &time_to_wait)))
      {
        //Timeout, exit thread
        pthread_mutex_unlock(&(pool->mtx));
        pthread_exit(NULL);
        return NULL;
      }
    //Get filename from queue
    strcpy(filename, pool->files[pool->head]);
    pool->head = (pool->head +1)%pool->file_count;
    pool->count -= 1;

    pthread_mutex_unlock(&(pool->mtx));

    //printf("\nReading %s", filename);

    read_file(filename);
  }

  pthread_mutex_unlock(&(pool->mtx));
  pthread_exit(NULL);
  return NULL;
}

/* Function that frees resources of thread_pool
 * object pointed to by given pointer
 * @param: pointer to thread_pool object
 */
int free_thread_pool(struct thread_pool *pool)
{
    if(pool == NULL)
    {
      return -1;
    }
    free(pool->threads);
    free(pool->files);
    pthread_mutex_lock(&(pool->mtx));
    pthread_mutex_destroy(&(pool->mtx));
    pthread_cond_destroy(&(pool->cnd));
    free(pool);
    return 0;
}

/* Function that allocates resources and initializes
 * an instance of the thread_pool
 * @param: num_threads: Number of threads in pool
 * @param: num_files: Max number of files in queue
 * @return: pointer to an instance of initialized thread_pool struct
 */
struct thread_pool *create_thread_pool(int num_threads, int num_files)
{
  int i;
  int ret;
  struct thread_pool *pool;

  if(num_threads < 0 || num_files < 0)
    goto error;

  if((pool = (struct thread_pool*)malloc(sizeof(struct thread_pool)))==NULL)
    goto error;

  pool->thread_count = num_threads;
  pool->file_count = num_files;
  pool->head = 0;
  pool->tail = 0;
  pool->count = 0;

  if((pool->files = malloc(num_files * sizeof(char*)))==NULL)
    goto error;

  for(i=0;i<num_files;i++)
    if((pool->files[i] = malloc(sizeof(char)*MAX))==NULL)
      goto error;

  if((pool->threads = (pthread_t *)malloc(sizeof(pthread_t)
          * num_threads))==NULL)
    goto error;
  if((pthread_mutex_init(&(pool->mtx), NULL))!=0)
    goto error;
  if((pthread_cond_init(&(pool->cnd), NULL))!=0)
    goto error;

  for(i=0;i<num_threads;i++)
  {
    if((pthread_create(&(pool->threads[i]), NULL,
            thread_run, (void*)pool))!=0)
      goto error;
  }
  return pool;
error:
  if(pool)
    free_thread_pool(pool);
  return NULL;

}

/* Function to add file to filenames queue for processing by threads
 * @param: pool: pointer to thread_pool object
 * @param: filename: string filename to add to queue
 */
void add_file(struct thread_pool *pool, char *filename)
{
  int next_index;
  int i;
  pthread_mutex_lock(&(pool->mtx));

  next_index = (pool->tail + 1) % pool->file_count;

  //Add filename to queue
  strcpy(pool->files[pool->tail], filename);
  pool->tail = next_index;
  pool->count += 1;


  pthread_cond_signal(&(pool->cnd));

  pthread_mutex_unlock(&(pool->mtx));
}
