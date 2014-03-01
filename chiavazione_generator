#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
int main(){
	
	// computing the first part of the "chiavazione
	double m;
	m = 300;
	double log10m;
	log10m = log10(m);

	int result = (int) ceil(log10m);
	
	//calculate a random alphanumeric pwd with length of 8 characters 
	
	char* pwd = "abcdefghilmnopqrstuvzABCDEFGHILMNOPQRSTUVZ1234567890";
	int i;
	int length;
	length = 52; //length of pwd

	//char* partial_pwd = malloc(8);
	
	char partial_pwd[8];	//pwd of max 8 characters

	srand(time(0));
	
	for( i = 0; i<8;i++){
		
		int random = rand()%length;		
		partial_pwd[i] = pwd[random];
	
	}
		partial_pwd[8] = '\0';
	

	
	
	
	//transforming the first part of chiavazione from integer to string
	char* convertion = malloc(20);
	sprintf(convertion,"%d",result);
	
	
	//appending the two calculated parts to create final form of chiavazione
	char* chiavazione;
	chiavazione = strcat(convertion,partial_pwd);
	printf("chiavazione: %s \n", chiavazione);
}
