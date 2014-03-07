#include "conversion.h"

unsigned int int_to_charc(unsigned int num){
	int i = 0;
	do{
		num = num / 10;
		i++;
	}while(num != 0);
	return i;
}
