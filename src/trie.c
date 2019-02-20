#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<ctype.h>
#define ALPHA 36 /*Number of letters and digits*/
#define MAX 10000 /*Maximum length of a word*/

//Struct defining a trie node
struct Trie
{
  struct Trie *children[ALPHA]; /* ALPHA number of children
                                   for every node */
  char *word; /* word at the end of this path */
  int is_word; /* Boolean integer denoting if there
                  is a word at the end of this path */
  int count; /* 0 is not a word, number of times this word has appeared */
  pthread_mutex_t mtx; /* Mutex for access to this trie node */
};

//Struct defining a word and its count.
//Definition: global list of words and their counts
struct word_count
{
  char word[MAX];
  int count;
}all[10];

//Index fot word_count global array
int k;

//Referring to global Trie
extern struct Trie *trie;

/*Function to recursively free entire trie
 */
void free_trie(struct Trie *t)
{
  if(t==NULL)
    return;
  free(t->word);
  pthread_mutex_destroy(&(t->mtx));
  int i;
  for(i=0;i<ALPHA;i++)
    free_trie(t->children[i]);

  free(t);

}

/* Function to allocate resources to create a trie node
 * @return: pointer to struct trie type object
 */
struct Trie *create_trie()
{
  struct Trie *t = (struct Trie *)malloc(sizeof(struct Trie));
  if(t==NULL)
    goto err;
  t->is_word = 0;
  int i;

  for(i=0;i<ALPHA;i++)
    t->children[i] = NULL;
  t->count = 0;
  if((t->word = (char *)malloc(sizeof(char)*MAX))==NULL)
    goto err;
  if((pthread_mutex_init(&(t->mtx), NULL)!=0))
      goto err;
  return t;
err:
  if(t)
    free_trie(t);
  return NULL;
}

/* Function the provides index in children array for the character
 * @param: x character
 * @return: position of x in children array a-z0-9.
 */
int get_index(char x)
{
  x = tolower(x);
  if((x >= 'a')&&(x <= 'z'))
  {
    return (x - 'a');
  }
  else
    return 26 + (x - '0');
}

/* Adds a word to trie
 * @param: t: pointer to Trie
 * @param: word: string word to add
 */
void add_to_trie(struct Trie *t, char *word)
{
  int i, ind;
  struct Trie *p = t;
  for(i=0;i<strlen(word);i++)
  {
    ind = get_index(word[i]);
    if(p->children[ind] == NULL)
      p->children[ind] = create_trie();
    p = p->children[ind];
  }
  pthread_mutex_lock(&(p->mtx));
  strcpy(p->word, word);
  p->is_word = 1;
  p->count += 1;
  //printf("\n%s %d *** ", p->word, p->count);
  pthread_mutex_unlock(&(p->mtx));
}


/* Recursive function to find words and counts in trie */
void recurse(struct Trie *p)
{
  int i, min, index;
  if(p == NULL)
    return;
  if(p->is_word)
  {
    //printf("\n ^^^ %s %d", p->word, p->count);
    if(k < 10)
    {
      strcpy(all[k].word, p->word);
      all[k++].count = p->count;
    }
    else
    {
      min = all[0].count;
      index = 0;
      for(i=0;i<10;i++)
      {
        if(all[i].count < min)
        {
          min = all[i].count;
          index = i;
        }
      }
      if(p->count > min)
      {
        all[index].count = p->count;
        strcpy(all[index].word, p->word);
      }
    }
  }
  for(i=0;i<ALPHA;i++)
    recurse(p->children[i]);
}

/* Function to traverse through trie to find words and counts */
void traverse(struct Trie *p)
{
  recurse(p);
}

/* Function that prints top n most occuring words to stdout */
void sort_top(int n)
{
  int i,j, max, index;
  for(j=0;j<n;j++)
  {
    max = all[0].count;
    index = 0;
    //printf("\n %d %s %d", k, all[0].word, all[0].count);
    for(i=1;i<k;i++)
    {
      if(max < all[i].count)
      {
        max = all[i].count;
        index = i;
      }
    }
    if(max > 0)
      printf("\n %s\t%d", all[index].word, max);
    else
    {
      printf("\n Not enough unique words. Required %d", n);
      break;
    }
    all[index].count = -1;
  }
}
