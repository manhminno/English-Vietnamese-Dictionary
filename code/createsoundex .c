#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"inc/btree.h"

static char code[128] = { 0 };

const char* soundex(const char *s)
{
	static char out[5];
	int c, prev, i;

	out[0] = out[4] = 0;
	if (!s || !*s) return out;

	out[0] = *s++;

	/* first letter, though not coded, can still affect next letter: Pfister */
	prev = code[(int)out[0]];
	for (i = 1; *s && i < 4; s++) {
		if ((c = code[(int) * s]) == prev) continue;

		if (c == -1) prev = 0;	/* vowel as separator */
		else if (c > 0) {
			out[i++] = c + '0';
			prev = c;
		}
	}
	while (i < 4) out[i++] = '0';
	return out;
}

void add_code(const char *s, int c)
{
	while (*s) {
		code[(int)*s] = code[0x20 ^ (int) * s] = c;
		s++;
	}
}

void init()
{
	static const char *cls[] =
	{ "AEIOU", "", "BFPV", "CGJKQSXZ", "DT", "L", "MN", "R", 0};
	int i;
	for (i = 0; cls[i]; i++)
		add_code(cls[i], i - 1);
}
int main()
{
	init();
	btinit();
	BTA *data, *soundexTree;
	char name[50], mean[1000];
	int rsize;
	char soundexcode[5];

	data = btopn("../data/english-vietnamese.dat",0,1);
	soundexTree = btcrt("../data/soundexTree.dat",0,1);
	btsel(data, "", mean, 100000 * sizeof(char), &rsize);
	while( btseln(data, name, mean, 100000 * sizeof(char), &rsize) == 0){
		strcpy(soundexcode, soundex(name));
		btins(soundexTree, name, soundexcode, 5 * sizeof(char));
	}
	btsel(soundexTree, "", soundexcode, 100000 * sizeof(char), &rsize);
	
	printf("Make soundexTree.dat successed !\n");
	btcls(data);
	btcls(soundexTree);
}
