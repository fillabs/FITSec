#include <stdio.h>
#include "fitsec_unstats.h"

int main(int argc, char ** argv){
	int i;
	printf("static unsigned short _un_stats_regions[] = {");
	for(i=0; i<sizeof(_un_stats_regions)/sizeof(_un_stats_regions[0]); i++){
		if(0 == (i&7)) printf("\n\t");
		printf("%4d, ", _un_stats_regions[i]);
	}
	printf("\n};\n");
	return 0;
}