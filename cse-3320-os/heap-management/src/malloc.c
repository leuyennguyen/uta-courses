#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)

static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};

struct _block *heapList = NULL; /* Free list to track the _blocks available */
struct _block *last_reused = NULL; /* Next-Fit algorithm */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
   /* Best fit */
   /* Initialize a temp pointer to keep track with the best-fit address space */
   struct _block *temp = NULL; 
   
   /* Traverse through the list */
   while (curr != NULL)
   {
      /* If a node, which has greater size than the requested size, is free
         and (either the temp ptr is NULL) or (the node's size is less than
         the size of previous qualified node), update the optimal node */
      /* (temp == NULL) is needed so that the program won't seg fault */
      if (curr->free && (curr->size >= size) && (temp == NULL || curr->size < temp->size))
      {
            temp = curr;
            /* If the space of most-fit space is found, stop */
            if (temp->size == size) break; 
      }
      *last = curr;
      curr = curr->next;
   }
   curr = temp;
#endif

#if defined WORST && WORST == 0
   /* Worst fit */
    /* Initialize a temp pointer to keep track with the worst-fit address space */
   struct _block *temp = NULL;
   while (curr != NULL)
   {
      /* If a node, which has greater size than the requested size, is free
         and (either the temp ptr is NULL) or (the node's size is greater than
         the size of previous qualified node), update the optimal node */
      if (curr->free && (curr->size >= size) && (temp == NULL || curr->size > temp->size))
      {
            temp = curr;
            /* If the space of most-fit space is found, stop */
            if (temp->size == size) break; 
      }
      *last = curr;
      curr = curr->next;
   }
   curr = temp;
#endif

#if defined NEXT && NEXT == 0
   /* Next fit */
   /* Traverse through the linked list to find where is previously left off */
   while (last_reused != NULL && !(last_reused->size >= size && last_reused->free))
   {
      *last = last_reused;
      last_reused = last_reused->next;
   }
   
   if (last_reused == NULL) last_reused = curr;
      
   while (last_reused != NULL && !(last_reused->size >= size && last_reused->free))
   {
      *last = last_reused;
      last_reused = last_reused->next;
   }
   
   curr = last_reused;
#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to prev _block */
   if (last) 
   {
      last->next = curr;
   }
   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{  
   /* Total amount of memory requested equals the current value plus
      the new requested size */
   num_requested += size;
   
   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   /* TODO: Split free _block if possible */

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
      
      /* Increment num_grows variable everytime growHeap() is called */
      num_grows++;
      
      /* Increment num_blocks variable everytime a new block is created.
         num_blocks = number of blocks in free list
         Assumming heapList is the mentioned free list. */
      num_blocks++;
      
      /* Maximum heap size is the current max size plus the added size */
      max_heap += size;
   }
   else
   {
      /* Increment num_reuses variable if we do not need to grow heap
         and reuse a block */
      num_reuses++;
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark _block as in use */
   next->free = false;
   
   /* If the user calls malloc() successfully, increment num_mallocs variable */
   num_mallocs++;

   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}

void *calloc(size_t num_blocks, size_t size)
{
   /* Initialize a new pointer */
   struct _block *ptr = NULL;
   
   /* If 0 is passed in, calloc() returns NULL */
   if (num_blocks == 0 || size == 0)
   {
      return NULL;
   }
   else
   {
      /* Otherwise, call malloc() to allocate space to handle the number of 
         n blocks of size m, in which n = num_blocks and m = size */
      ptr = malloc(num_blocks * size);
      
      /* If malloc() fails, calloc() fails */
      if (ptr == NULL)
      {
         return NULL;
      }
      else
      {
         /* Otherwise, memset() all memory to 0 and return */
         memset(ptr, 0, num_blocks*size);
         return ptr;
      }
   }
}

void *realloc(void *ptr, size_t size)
{
   /* If ptr is NULL, realloc(NULL, size) becomes malloc(size) */
   if (ptr == NULL)
   {
      return malloc(size);
   }
   else
   {
      /* Otherwise, get the size of the argument ptr, initialize new ptr,
         and malloc() on that new ptr. If malloc() fails, realloc() fails.
         If not, deallocate the old object pointing to ptr, free ptr, and
         return. */
      struct _block *temp = BLOCK_HEADER(ptr);
      struct _block *new_ptr = NULL;
      new_ptr = malloc(temp->size);
      if (new_ptr == NULL)
      {
         return NULL;
      }
      else
      {
         memcpy(new_ptr, (struct _block *) ptr, temp->size);
         free(ptr);
         return new_ptr;
      }
   }
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }
   /* If the user calls free() successfully, increment num_frees variable*/
   num_frees++;
   
   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;

   /* TODO: Coalesce free _blocks if needed */
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
