#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
void sighandler(int sig)
{
	printf("..%d\n",sig);
}
 
int main(int argc, char* argv[])
{
 
	if(signal(SIGINT, sighandler) == SIG_ERR)
	{
		printf("hello world\n");
	}
	while(1)
	{
		printf("sleep...\n");
		sleep(1);
	}
 
	return 0;
}
