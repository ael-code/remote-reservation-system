#include "conversion.h"

int int_to_charc(unsigned int num){
	int i = 0;
	do{
		//printf("iter: %d -- %d\n",i,num);
		num = num / 10;
		i++;
	}while(num != 0);
	return i;
}
