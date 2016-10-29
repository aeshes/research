#include "hash.h"

char *strdup(const char *s)
{
	char *p = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlen(s) + 1);
	if (p != NULL)
		lstrcpy(p, s);
	return p;
}

#define EQKEY(s1, s2)	(lstrcmp(s1, s2) == 0)
#define SETKEY(dst, src) (dst = strdup(src))

int hashfunc(char *key)
{
	unsigned int hash = 0;
	while (*key)
	{
		hash = (hash << 1) | (hash >> 15);	/* ROL */
		hash ^= *key++;
	}
	hash = hash % HASHSIZE;
	return hash;
}

struct cell *find(char *key)
{
	struct cell *p;
	for (p = hashtable[hashfunc(key)]; p; p = p->next)
	{
		if (EQKEY(p->ext, key))
			return p;
	}
	return NULL;
}

void insert(char *key)
{
	struct cell *p;
	if ((p = find(key)) == NULL)
	{
		if (!(p = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct cell)))) return;
		SETKEY(p->ext, key);
		p->next = hashtable[hashfunc(key)];
		hashtable[hashfunc(key)] = p;
	}
}

void printcell(struct cell *ptr)
{
	putchar('(');
	printf("%s", ptr->ext); putchar(')');
	putchar('\n');
}

void printtable(void){
	register int i; struct cell *p;
	printf("----TABLE CONTENTS----\n");
	for(i = 0; i < HASHSIZE; i++)
		if((p = hashtable[i]) != NULL)
		{
			printf("%d: ", i);
			for(; p; p = p->next)
				printcell(p), putchar(' ');
			putchar('\n');
		}
}

void freetable(void)
{
	struct cell *current, *next;
	register int i;
	for (i = 0; i < HASHSIZE; i++)
	{
		if (hashtable[i] != NULL)
		{
			current = hashtable[i];
			next = current->next;
			while (current)
			{
				HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, current);
				current = next;
				if (next) next = next->next;
			}
			hashtable[i] = NULL;
		}
	}
}
