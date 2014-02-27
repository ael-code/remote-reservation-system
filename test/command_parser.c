#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char help[200] = "\n\
Usage: parser\n\
\n\
-c --colored  Show colors in output\n\
-h --help     Show this screen.\n\
-v --version  Print program version.\n\
";

char helpColor[200] = "\n\
Usage: parser\n\
\n\
-c --colored  \e[0;34mShow colors in output.\e[0m\n\
-h --help     \e[0;34mShow this screen.\e[0m\n\
-v --version  \e[0;34mPrint program version.\e[0m\n\
";

int color = 0;

int main (int argc, char ** argv){
	//da usare argp
	int opt;
	while((opt = getopt(argc,argv,"cvh")) != -1){
		switch(opt){
			case 'c':
				color = 1;
			break;
			case 'h':
				if(color)
					printf("%s\n",helpColor);
				else
					printf("%s\n",help);
			break;
			case 'v':
				printf("non stamo manco alla prima\n");
			break;
		}
	}
	exit(0);
}
