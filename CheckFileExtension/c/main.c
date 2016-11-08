#include <stdio.h>
#include "hash.h"


int main(int argc, char *argv[])
{
	insert("doc");
	insert("exe");
	insert("txt");

	printtable();
	freetable();
	printtable();
}
