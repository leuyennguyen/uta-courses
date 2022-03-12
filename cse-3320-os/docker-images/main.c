// Le Uyen Nguyen - 100 171 8086

#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void handle_alarm(int sig)
{
	printf("Caught alarm!\n");
	alarm(1);
}

int main(void)
{	
	struct sigaction act;
	
	memset(&act, '\0', sizeof(act));
	
	act.sa_handler = &handle_alarm;
	
	if (sigaction(SIGALRM, &act, NULL) < 0)
	{
		perror("sigaction: ");
		return EXIT_FAILURE;
	}
	
	pid_t pid = fork();
	
	// fork() returns -1 when it is unable to create a process
	if (pid == -1)
	{
		perror("fork failed: ");
		exit(EXIT_FAILURE);
	}
	
	// When fork() returns 0, we are in the child process
	else if (pid == 0)
	{
		struct timeval begin;
		struct timeval end;
		
		gettimeofday(&begin, NULL);
		gettimeofday(&end, NULL);
		
		int elapsed_time = end.tv_sec - begin.tv_sec;
		
		while (elapsed_time < 10)
		{
			gettimeofday(&end, NULL);
			elapsed_time = (end.tv_sec + (end.tv_usec/1000000)) - 
					(begin.tv_sec + (begin.tv_usec/1000000));
	
	
			// Print the elapsed time from 10 to 0 to console
			printf("%d\n", 10 - elapsed_time);
			sleep(1);
		}
		fflush(NULL);
		exit(EXIT_SUCCESS);
	}
	
	// When fork() returns a positive number, we are in the
	// parent process and the return value is the PID of 
	// the newly created child process
	else
	{
		int status;
		waitpid(pid, &status, 0);
		printf("Countdown complete.\n");
		fflush(NULL);
	}
	
	alarm(1);
	
	/*
	while (1)
		sleep(1)
	*/
	
	return EXIT_SUCCESS;
}

