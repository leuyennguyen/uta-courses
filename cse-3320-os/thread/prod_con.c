/*
	Name: Le Uyen Nguyen
	ID: 100 171 8086
	gcc -pthread prod_con.c
	./a.out <file_name>

*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include  <string.h>

#define BUFFER_SIZE 5

char message[BUFFER_SIZE]; // The circular queue storing message
int front = 0, rear = 0; // Head and tail of the circular queue

// The capacity of the circular queue, which 
// have max size of BUFFER_SIZE
int capacity = 0;

int eof_flag = 0; // The end-of-file flag signaling threads when to stop

pthread_mutex_t mutex;
// The condition for the consumer reads/deletes character from the
// message. The producer will wait for an available slot in the queue
pthread_cond_t overflow;  
// The condition for the producer to put the character from file 
// to queue. The consumer will wait for new information to read.
pthread_cond_t underflow;

void Producer(FILE *FileHandle)
{
    int ch;
    ch = fgetc(FileHandle);

    while (1)
    {
    	pthread_mutex_lock(&mutex);
    	// If the queue's capacity is greater than or equal BUFFER_SIZE,
    	// there is no space for producer to put character in. It then
    	// must wait for the consumer to clear up some slots in buffer.
    	if (capacity >= BUFFER_SIZE)
    	{
    		pthread_cond_wait(&overflow, &mutex);
    	}
    	message[rear++] = (char) ch;
    	rear %= BUFFER_SIZE; // Guarantee rear value is within the queue's max size
    	capacity++; // Update queue's capacity
    	pthread_cond_signal(&underflow);
    	pthread_mutex_unlock(&mutex);
    	ch = fgetc(FileHandle);
    	
    	// If eof is reached, eof_flag is updated and producer stops.
    	if (feof(FileHandle))
    	{
    		pthread_mutex_lock(&mutex);
    		eof_flag = 1;
    		pthread_cond_signal(&underflow);
    		pthread_mutex_unlock(&mutex);
    		break;
    	}
    }
}         

void *Consumer(void *ptr) 
{
	while (1)
	{
		pthread_mutex_lock(&mutex);
		// If the queue's capacity is less than or equal 0, there is no
    		// new character to read. The consumer then must wait for the
    		// producer to put something in.
		if (capacity <= 0)
		{
			// If eof_flag = 0, there is stil available information to read
			if (!eof_flag) 
			{
				pthread_cond_wait(&underflow, &mutex);
			}
			// else, the file is end; eof_flag = 1, consumer stops
			else break;
		}
		printf("%c", message[front++]);
		front %= BUFFER_SIZE; // Guarantee front value is within the queue's max size
		capacity--; // Update the queue's capacity
		pthread_cond_signal(&overflow);
    		pthread_mutex_unlock(&mutex);
	}
}

int main(int argc, char *argv[]) 
{
	FILE *FileHandle;
	char filename[50] = {}; // Assume the file name is at most 50 characters
	
	if (argc < 2)  // Use default file "message.txt"
	{
		strcpy(filename, "message.txt");
	}
	else // Use the file provided at command line
	{
		strcpy(filename, argv[1]);
	}
	
    	if ((FileHandle = fopen((char *) filename, "r")) == NULL)
    	{
        	printf("ERROR: can’t open %s!\n", filename);
        	exit(0);
    	}
    
	pthread_t producer, consumer;
	
	pthread_mutex_init(&mutex, NULL);
	
	pthread_cond_init(&overflow, NULL);
	pthread_cond_init(&underflow, NULL);
	
	if (pthread_create(&producer, NULL, (void *) &Producer, (FILE *) FileHandle))
	{
		printf("\n ERROR: unable to create producer thread\n");
		exit(1);
	}
	
	if (pthread_create(&consumer, NULL, Consumer, NULL))
	{
		printf("\n ERROR: unable to create consumer thread\n");
		exit(1);
	}
	
	if (pthread_join(producer, NULL))
	{
		printf("\n ERROR: unable to join thread\n");
		exit(1);
	}
	
	if (pthread_join(consumer, NULL))
	{
		printf("\n ERROR: unable to join thread\n");
		exit(1);
	}	
	
	pthread_exit(NULL);
	
	fclose(FileHandle);

	return 1;
}
