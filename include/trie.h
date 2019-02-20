#ifndef TRIE_H
#define TRIE_H

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<ctype.h>
#include "trie.h"
#define ALPHA 36 /*Number of letters and digits*/
#define MAX 10000 /*Maximum length of a word*/
//Struct defining a trie node
struct Trie;

//Struct defining a word and its count. Definition: global list of
//words and their counts
struct word_count;

/* Function to allocate resources to create a trie node
 * @return: pointer to struct trie type object
 */
struct Trie *create_trie();

/*Function to recursively free entire trie
 */
void free_trie(struct Trie *t);

/* Adds a word to trie
 * @param: t: pointer to trie
 * @param: word: string word to add
 */
void add_to_trie(struct Trie *t, char *word);

/* Function to traverse through trie to find words and counts */
void traverse(struct Trie *p);

/* Function that prints top n most occuring words to stdout */
void sort_top(int n);
#endif
