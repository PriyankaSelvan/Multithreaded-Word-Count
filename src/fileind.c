/*
 * fileind.c - Main thread of the program that searches
 *             directories for .txt files
 *             Hands them over to worker thread
 *             Traverses trie to get all counts
 *             Displays 10 most occuring words
 */

#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<dirent.h>
#include<string.h>
#include<unistd.h>
#include <sys/stat.h>
#include "pool.h"

#define MAX_TASKS 1000 /*Max number of tasks in queue*/

//Global thread pool and trie
struct thread_pool *tp;
struct Trie *trie;


int number_threads;

/* Function to check if string is a directory
 * @param : path: path string
 * @return: 1 if directory exists 0 otherwise
 */
int isdir(char* path)
{
  struct stat buf;
  stat(path, &buf);
  return S_ISDIR(buf.st_mode);
}

/*Function to check if extension is .txt
 * @param : name : filename string
 * @return: 1 if .txt file 0 otherwise
 */
int istxtfile(char *name)
{
  int len = strlen(name);
  return ((len > 3)&&(name[len-4] == '.')&&(name[len-3] == 't')&&
         (name[len-2] == 'x')&&(name[len-1] == 't'));
}

/*Function to recursively get files. When a .txt file is found,
 * it is added to the task queue
 * @param: Path: string path to search in
 */
void get_files(char *Path)
{
  char path[MAX];
  char filename[MAX];
  struct dirent *dp;
  DIR *dir = opendir(Path);

  // Unable to open directory stream
  if (!dir)
    return;


  while ((dp = readdir(dir)) != NULL)
  {
    if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
    {
      strcpy(filename, dp->d_name);

      // Construct new path from our base path
      strcpy(path, Path);
      strcat(path, "/");
      strcat(path, dp->d_name);
      if(istxtfile(filename))
      {
        //printf("\nFound %s", path);
        add_file(tp, path);
      }

      get_files(path);
    }
  }

  closedir(dir);
}

int main(int argc,char* argv[])
{
  int counter;
  char error[] = "\n Incorrect usage. Use './ssfi --help' for help\n";
  char help[] = "\n Usage: ./ssfi -t number_of_threads path_to_search\n";
  char folder[MAX];

  //CLI args handling
  if(argc == 2)
  {
    //should be help option
    if(strcmp(argv[1], "--help") == 0)
    {
      printf("%s", help);
      exit(0);
    }
    else
    {
      printf("%s", error);
      exit(EXIT_FAILURE);
    }
  }
  else if(argc == 4)
  {
    counter = 1;
    while (counter < argc)
    {
      if(argv[counter][0] == '-')
      {
        switch (argv[counter][1])
        {
          case 't':
           if((number_threads=atoi(argv[counter+1]))&&(number_threads>0));
           else{printf("%s", error);exit(EXIT_FAILURE);} ; break;
          default:
            printf("%s", error);
            exit(EXIT_FAILURE);
        }
        counter += 2;
      }
      else
      {
        strcpy(folder, argv[counter]);
        counter += 1;
      }
    }

  }
  else
  {
    printf("%s", error);
    exit(EXIT_FAILURE);
  }


  tp = create_thread_pool(number_threads, MAX_TASKS);

  //Creating the global trie
  trie = create_trie();

  //Start looking for .txt files
  if(isdir(folder))
    get_files(folder);
  else
  {
    printf("\nPath to look in is invalid. Has to be a directory\n");
    exit(EXIT_FAILURE);
  }

  //Wait for all threads to exit
  for(counter = 0;counter < number_threads; counter++)
  {
    pthread_join(tp->threads[counter], NULL);
  }

  //Traverse trie to get words and their counts
  traverse(trie);

  //Get top 10 printed to stdout
  sort_top(10);

  free_thread_pool(tp);
  free_trie(trie);


  printf("\n");

  return 0;
}
