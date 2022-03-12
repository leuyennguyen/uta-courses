/*
	Le Uyen Nguyen
	100 171 8086
	gcc -pthread thread.c
	./a.out <file_name>
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX 5000000
#define NUM_THREADS 2

int total = 0;
// The total number of substrings
int total_substring = 0;
// n1 and n2 are lengths of string s1 and s2
int n1, n2; 
// s1 and s2 are the two strings read from file
char *s1, *s2;

FILE *fp;

// String s1 is evenly partitioned for NUM_THREADS
// to concurrently search for matching with string s2
int partition = 0;

pthread_mutex_t mutex;

int readf(char* filename)
{
    if ((fp=fopen(filename, "r")) == NULL)
    {
        printf("ERROR: can’t open %s!\n", filename);
        return 0;
    }
    
    s1 = (char *)malloc(sizeof(char)*MAX);
    
    if (s1 == NULL)
    {
        printf ("ERROR: Out of memory!\n") ;
        return -1;
    }
    
    s2 = (char *)malloc(sizeof(char)*MAX);
    
    if (s1 == NULL)
    {
        printf ("ERROR: Out of memory\n") ;
        return -1;
    }
    
    /*r ead s1 s2 from the file */
    s1 = fgets(s1, MAX, fp);
    s2 = fgets(s2, MAX, fp);
    n1 = strlen(s1); /* length of s1 */
    n2 = strlen(s2)-1; /* length of s2 */
    
    if (s1 == NULL || s2 == NULL || n1 < n2) /* when error exit*/
    {
        return -1;
    }
}

int num_substring ( void )
{
    int i,j,k;
    int count ;
    for (i = 0; i <= (n1-n2); i++)
    {
        count =0;
        for(j = i ,k = 0; k < n2; j++,k++)
        { //search for the next string of size of n2
            if (*(s1+j)!=*(s2+k))
            {
                break ;
            }
            else
            {
                count++;
            }
            if (count==n2)
                total++; //find a substring in this step
         }
    }
    return total ;
}

void num_substring_parallel(void *ptr)
{
	int start =  (int) ptr;
    int i, j, k;
    int count;
    pthread_mutex_lock(&mutex);
    for (i = start ; i <= (start+partition) ; i++)
    {
        count = 0;
        for (j = i, k = 0 ; k < n2 ; j++, k++)
        { /* search for the next string of size of n2 */
            if (*(s1+j)!=*(s2+k))
            {
                break ;
            }
            else
            {
                count++;
            }
            
            if (count == n2)
            {
                total_substring++; /* find a substring in this step */
         	}
         }
    }
	pthread_mutex_unlock(&mutex);
}
    
int main(int argc, char *argv[])
{
	int count;
    if (argc < 2)
    {
      printf("Error: You must pass in the datafile as a commandline parameter\n");
    }

    if (readf(argv[1]) == 0)
    { 
    	return EXIT_FAILURE;
    }
    
    partition = n1 / NUM_THREADS;
    
    // The number of threads concurrently working on file
    pthread_t threads[NUM_THREADS];
    
    pthread_mutex_init(&mutex, NULL);
    
    int i;
    for (i = 0 ; i < NUM_THREADS ; i++)
    {
    	// The 1st thread will execute function num_substring_parallel() from character
    	// 0 to character at partition-th position. The next thread will work on
    	// subtring at position (i*partition) to (i*partition)+partition position.
    	// All threads work on the strings, all of which have the same length.
    	
    	int starting_point = i * partition; // The position in the string where the thread
    										// start working on
    	
    	if (pthread_create(&threads[i], NULL, (void *) &num_substring_parallel, (void *) starting_point))
    	{
    		printf("\n ERROR: unable to create thread\n");
    		exit(1);
    	}
    }

    for (i = 0 ; i <  NUM_THREADS ; i++)
    {
    	if (pthread_join(threads[i], NULL))
    	{
    		printf("\n ERROR: unable to join thread\n");
    		exit(1);
    	}
    }
    
    count = num_substring();
    
    struct timeval start, end;
    float mtime; 
    int secs, usecs;    

    gettimeofday(&start, NULL);

    gettimeofday(&end, NULL);

    secs  = end.tv_sec  - start.tv_sec;
    usecs = end.tv_usec - start.tv_usec;
    mtime = ((secs) * 1000 + usecs/1000.0) + 0.5;

	printf("The number of substrings in sequence is : %d\n" , count);
    printf ("The number of substrings in parallel is : %d\n" , total_substring);
    printf ("Elapsed time is : %f milliseconds\n", mtime );

    if( s1 )
    {
      free( s1 );
    }

    if( s2 )
    {
      free( s2 );
    }
    
	fclose(fp);
	
    return 0 ; 
}

