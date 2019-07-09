#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"inc/btree.h"

void fixName(char *name);
void processString(char *input, char *output);

int main() {

	BTA *data;
	FILE* fo;
	int index, i=0, count = 0;
	char name[50], mean[100000], tmp[100];
	data = btcrt( "../data/english-vietnamese.dat", 0, 0);
	fo = fopen("../data/english-vietnamese.txt", "r");
	if (fo == NULL) {
		printf("Error\n");
		exit(EXIT_FAILURE);
	}
	while(!feof(fo)){
		fgets(tmp, 100, fo);
		while( tmp[0] != '@'){
			strcat(mean, tmp);
			if( feof(fo)) break;
			fgets(tmp, 100, fo);
		}
		int k = btins(data, name, mean, 1000 * sizeof(char));
		if(k == 0) count++;
		if(tmp[0] == '@'){
			strcpy(name, tmp);
			mean[0] = '\0';
			processString(name, mean);
			fixName(name);
		}
	}
	printf("Insert %d word into file english-vietnamese.dat\n", count);
	fclose(fo);
	btcls(data);
}

void fixName(char *name){
	int length = strlen(name);
	for(int i = 1; i <= length; i++) {
		name[i-1] = name[i];
	}
}
void processString(char *word, char *mean) {
	int i, j, k=0, check = 0;
	int length = strlen(word);
	for(i=0; i < length; i++) {
		if(word[i] == '/'){
			check = 1;
			break;
		}
	}
	if(check == 0) {
		word[length - 1] = '\0';
		return;
	}
	strcpy(mean, word);
	mean[i-1] = '\n';
	word[i-1] = '\0';
}
