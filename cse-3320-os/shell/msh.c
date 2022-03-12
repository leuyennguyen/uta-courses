/*
	Name: Le Uyen Nguyen
	ID: 100 171 8086
*/

// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017, 2021 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 7f704d5f-9811-4b91-a918-57c1bb646b70
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11    // Mav shell only supports ten arguments

#define MAX_PROCESSES_SHOWED 15 // Mav shell stores the list of 15 processes

// All processes are stored in a queue
// Each node of the queue contains the process's pid,
// the char pointer to command's name, and pointer to
// the next node, containing following command.
struct pid_queue_array
{
	pid_t pid;
	char *command;
	struct pid_queue_array *next_ptr;
};

typedef struct pid_queue_array * ListPids;

// The shell only keeps up to MAX_PROCESSES_SHOWED,
// when queue reaches its limit, Dequeue is called to
// remove the head of the queue, and decrement its size.
// Mode 1 is to dequeue 1 node; otherwise, free all nodes.
void Dequeue(ListPids *Head, int *size, int mode);

// When the process is successfully created and the 
// valid command is entered, it will be added to the queue.
// The Head and Tail of the queue, pid, the command, and the 
// queue's size will be passed in Enqueue().
void Enqueue(ListPids *Head, ListPids *Tail, pid_t pid, char *root, int *size);

// Function GetCommandFromHistory takes the queue and
// "!n" command as arguments, and returns the command's
// name at the corresponding node.
char *GetCommandFromHistory(ListPids Head, int index);

// Function History takes the queue, and an integer representing 
// print mode. If print_mode = 0, the function will print the list
// of MAX_PROCESSES_SHOWED processes (supporting history command);
// otherwise, it will print the list of MAX_PROCESSES_SHOWED
// process PIDs (supporting listpids command).
void History(ListPids Head, int print_mode);

void Dequeue(ListPids *Head, int *size, int mode)
{
	ListPids Temp;
	
	if (mode == 1)
	{
		Temp = (*Head)->next_ptr;
		free((*Head)->command);
		free(*Head);
		*Head = Temp;
		(*size)--;
	}
	else
	{
		while (*Head != NULL)
		{
			Temp = (*Head)->next_ptr;
			free((*Head)->command);
			free(*Head);
			*Head = Temp;
		}
	}
}

void Enqueue(ListPids *Head, ListPids *Tail, pid_t pid, char *root, int *size)
{
	ListPids Process = malloc(sizeof(struct pid_queue_array));
	Process->pid = pid;
	Process->command = malloc(strlen(root)*sizeof(char)+1);
	strcpy(Process->command, root);
	Process->next_ptr = NULL;
	
	if (*Head == NULL)
	{
		*Head = *Tail = Process;
	}
	else
	{
		if (*size > MAX_PROCESSES_SHOWED-1)
		{
			// Dequeue is called inside Enqueue to ensure the
			// queue only contains MAX_PROCESSES_SHOWED processes
			// Mode 1 is passed to dequeue only 1 node
			Dequeue(Head, size, 1);
		}
		(*Tail)->next_ptr = Process;
		*Tail = Process;
	}
	(*size)++;
}

char *GetCommandFromHistory(ListPids Head, int index)
{
	ListPids Temp = Head;
	int tracking = 0;
	
	while (Temp != NULL)
	{
		if (index == tracking)
		{
			return Temp->command;
		}
		Temp = Temp->next_ptr;
		tracking++;
	}
	return NULL;
}

void History(ListPids Head, int print_mode)
{
	ListPids Temp = Head;
	
	if (print_mode == 0)	// User types history command
	{
		int index = 0;
		while (Temp != NULL)
		{
			printf("[%d]: %s", index, Temp->command);
			Temp = Temp->next_ptr;
			index++;
		}
	}
	else	// User types listpids command
	{
		while (Temp != NULL)
		{
			printf("%d\n", Temp->pid);
			Temp = Temp->next_ptr;
		}
	}
}

int main(int argc, char *argv[])
{
	char *cmd_str = (char*)malloc(MAX_COMMAND_SIZE);
	
	ListPids Head = NULL;
	ListPids Tail = NULL;
	int flag;	// flag decides if the valid command supported
				// by Mavs shell (execvp()) is added to the queue
	int size = 0;	// Size of the queue, limited by MAX_PROCESSES_SHOWED

	while (1)
  	{
  		flag = 1;	// Assuming that the following command is valid and supported
  		
    	// Print out the msh prompt
		printf ("msh> ");

		// Read the command from the commandline.  The
		// maximum command that will be read is MAX_COMMAND_SIZE
		// This while command will wait here until the user
		// inputs something since fgets returns NULL when there
		// is no input
		while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));
		
		// Hanlde !n command. 
		if (cmd_str[0] == '!')
		{
			// The index of executed command. Ranged from
			// 0 < command_index < MAX_PROCESSES_SHOWED
			int command_index = -1;
			
			if (cmd_str[1] >=48 && cmd_str[1] <= 57)	// "!xy" - check if x is valid
			{
				if (cmd_str[2] >=48 && cmd_str[2] <= 53)	//"!xy" - check if y is valid
				{
					command_index = ((int) cmd_str[2] - 48) + 10;	// 10 <= command_index < 15
				}
				else
				{
					command_index = (int) cmd_str[1] - 48;	// 0 < command_index <= 9
				}
			}
			
			if (command_index < size && command_index > -1)
			{
				char *duplicate = GetCommandFromHistory(Head, command_index);
				if (duplicate != NULL)
				{
					strcpy(cmd_str, duplicate);
				}
			}
			else
			{
				printf("Command not in history.\n");
				continue;
			}
		}
		
		/* Parse input */
		char *token[MAX_NUM_ARGUMENTS];
		
		int token_count = 0;
		
		// Pointer to point to the token parsed by strsep()
		char *argument_ptr;
		
		char *working_str = strdup(cmd_str);
		
		// we are going to move the working_str pointer so
		// keep track of its original value so we can deallocate
		// the correct amount at the end
		char *working_root = working_str;
		
		// Tokenize the input strings with whitespace used as the delimiter
		while (((argument_ptr = strsep(&working_str, WHITESPACE)) != NULL) && 
			(token_count < MAX_NUM_ARGUMENTS-1))
		{
			token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
			
			if (strlen(token[token_count]) == 0 )
			{
				token[token_count] = NULL;
			}
			
			token_count++;
		}
      	
		// Making char *token[] a NULL-terminated array because
		// execvp() takes the NULL-terminated array of arguments
		// to search for new process
		token[MAX_NUM_ARGUMENTS-1] = NULL;
      	
		/*
		// Now print the tokenized input as a debug check
		// \TODO Remove this code and replace with your shell functionality
		int token_index  = 0;
		for( token_index = 0; token_index < token_count; token_index ++ ) 
		{
			printf("token[%d] = %s\n", token_index, token[token_index] );
		}
		*/
		
		// The shell will exit if the user enter "quit" or "exit"
		// token[0] != NULL when the user gives valid input (not pressing enter)
		if (token[0] != NULL)
		{
			if ((strcmp(token[0], "quit") == 0) || (strcmp(token[0], "exit") == 0))
			{
				free(cmd_str);
				free(working_str);
				free(working_root);
				int i;
				for (i = 0 ; i < token_count ; i++)
				{
					free(token[i]);
				}
				Dequeue(&Head, &size, 0);
				return 0;
			}

			// chdir() executes cd command, which is to change the current
			// working directory. The continue command after chdir() is
			// to stop the program from parsing the following commands.
			// Since the subshell is created by fork() and then returns
			// to parent shell, the current directory will not be changed,
			// which fails chdir()
			else if (!strcmp(token[0], "cd"))
			{
				int ret = chdir(token[1]);
				if (ret < 0)
				{
					perror(token[1]);
				}
				else
				{
					Enqueue(&Head, &Tail, getpid(), cmd_str, &size);
				}
				continue;
			}
			else if (!strcmp(token[0], "history"))
			{
				Enqueue(&Head, &Tail, getpid(), cmd_str, &size);
				History(Head, 0);
				continue;
			}
			else if (!strcmp(token[0], "listpids"))
			{
				Enqueue(&Head, &Tail, getpid(), cmd_str, &size);
				History(Head, 1);
				continue;
			}
			
			int status;
			pid_t pid = fork();

			if (pid == 0)
			{
				int ret = execvp(token[0], &token[0]);
				
				if (ret == -1 && (strcmp(token[0], "cd") 
							|| strcmp(token[0], "history") 
							|| strcmp(token[0], "listpids")))
				{
					flag = 0;
					printf("%s: Command not found.\n", token[0]);
					exit(0);
				}
			}
			else
			{
				wait(&status);
				if (flag == 1)
				{
					Enqueue(&Head, &Tail, pid, cmd_str, &size);
				}
				fflush(NULL);
			}
		}
		free(working_root);
	}
	return 0;
}
