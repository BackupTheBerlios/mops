/***********************************************************
 * Standard C String helpful functions
 * $Id: string_operations.cpp,v 1.1 2007/01/19 06:13:59 i27249 Exp $
 * ********************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
char * strMerge(const char *part1, const char *part2)
{
#ifdef DEBUG
	printf("part 1: [%s]\n", part1);
	printf("part 2: [%s]\n", part2);
#endif
	int p1=strlen(part1);
	int p2=strlen(part2);
	// Beginning dump
	char *ret=(char *) malloc(p1+p2+1);
	for (int i=0; i<p1; i++)
	{
		ret[i]=part1[i];
	}
	for (int i=p1; i<=p1+p2; i++)
	{
		ret[i]=part2[i-p1];
	}
#ifdef DEBUG
	printf("ret: [%s]\n", ret);
#endif
	return ret;
}

/*
int main()
{
	char *c1="void";
	char *c2="matrix";
	char *c3=strMerge(c1, c2);
	printf("c3 = [%s]\n", c3);
	free(c3);
	return 0;
}
*/
