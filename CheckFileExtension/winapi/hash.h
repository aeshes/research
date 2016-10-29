#ifndef HASH_H
#define HASH_H

#include <windows.h>
#include <stdio.h>

#define HASHSIZE	64

struct cell
{
	char *ext;	/* extension */
	struct cell *next;	/* pointer to next element */
} *hashtable[HASHSIZE];

int hashfunc(char *key);
struct cell *find(char *key);
void insert(char *key);
void printcell(struct cell *ptr);
void printtable(void);
void freetable(void);


#endif
