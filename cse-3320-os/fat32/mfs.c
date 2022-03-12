/*
	Le Uyen Nguyen
	100 171 8086
*/

// The MIT License (MIT)
// 
// Copyright (c) 2020 Trevor Bakker 
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
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <libgen.h>

#define MAX_NUM_ARGUMENTS 4

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

/* FAT32 LAYOUT 
	- BPB_RsvdSecCnt * BPB_BytsPerSec in size
	- FAT #1 starts at address BPB_RsvdSecCnt * BPB_BytsPerSec
	- Each FAT has 1 32-bit word for every cluster. 
	Each entry is the logical block of the next block in the file. 
	- Total FAT size is BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec
	- Clusters start at address:
	(BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) +(BPB_RsvdSecCnt * BPB_BytsPerSec)
	- Clusters are each (BPB_SecPerClus * BPB_BytsPerSec) in bytes
	- Root Directory is at the first cluster. Address:
	(BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) +(BPB_RsvdSecCnt * BPB_BytsPerSec)
*/

struct __attribute__((__packed__)) DirectoryEntry
{
	char DIR_Name[11];
	uint8_t DIR_Attr;
	uint8_t Unused1[8];
	uint16_t DIR_FirstClusterHigh;
	uint8_t Unused2[4];
	uint16_t DIR_FirstClusterLow;
	uint32_t DIR_FileSize;
};

char BS_OEMName[8];
int16_t BPB_BytsPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int16_t BPB_RootEntCnt;
char BS_VolLab[11];
int32_t BPB_FATSz32;
int32_t BPB_RootClus;

int32_t RootDirSectors = 0;
int32_t FirstDataSector = 0;
int32_t FirstSectorofCluster = 0;

int32_t currentDirectory;

struct DirectoryEntry dir[16];

FILE * FileHandle;

/*
 * Function		: LBAToOffset
 * Parameters	: The current sector number that points to a block of data
 * Returns		: The value of the address for that block of data
 * Description	: Find the starting address of a block of data given the sector number
 * corresponding to that data block
 */
int LBAToOffset(int32_t sector)
{
	 return ((sector - 2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt)
				+ (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

/*
 * Function		: NextLB
 * Purpose		: Given a logical block address, look up into the first FAT and
 * return the logical block address of the block in the file. If there is no 
 * further blocks then return -1
 */
int16_t NextLB(uint32_t sector)
{
	uint32_t FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (sector * 4);
	int16_t val;
	fseek(FileHandle, FATAddress, SEEK_SET);
	fread(&val, 2, 1, FileHandle);
	return val;
}

// Function compare() takes the input filename, and returns the correctly 
// formatted file name that matches with ones in the image system.
char * compare(char *input)
{
	char *correctFilename = (char *) malloc(12);
	char expanded_name[12];
	memset(expanded_name, ' ', 12 );
	
	char *token = strtok(input, ".");
	
	strncpy(expanded_name, token, strlen(token));
		
	token = strtok(NULL, ".");
	if(token)
	{
		strncpy((char*)(expanded_name+8), token, strlen(token));
	}

	expanded_name[11] = '\0';
	
	int i;
	for (i = 0; i < 11; i++)
	{
		expanded_name[i] = toupper(expanded_name[i]);
	}
	
	strncpy(correctFilename, expanded_name, 12);
	return correctFilename;
}

/************************************
 *	FUNCTION DEFINITIONS		
 ************************************/

// ClusterLow() takes the filename, and returns its corresponding cluster
int32_t ClusterLow(char *);
// ClusterSize() take the cluster, and returns its size
int32_t ClusterSize(int32_t);
// Open the file image
void open(char *);
// info() prints out information about the file system in both decimal and hexadecimal
void info();
// stat() prints the file size, starting cluster #, and attributes of given input.
// If the input file/directory does not exits, print Error
void stat(char *);
// get() retrieves the file from the system image, and places it in the cwd
// If file/directory does not exist, print Error
void get(char *);
// cd() changes the cwd to the given directory
void cd(char *path);
// checkPath() checks if the given path is relative or absolute path
// (that is, contains '/'). If yes, it parses the directory separately
// and passes to cd(). If no, it calls cd() directly.
void checkPath(char *);
// ls() lists the directory contents. Only ".." is supported.
void ls();
// read() reads from the given file from the given position in bytes.
void read(char *, int, int);

/************************************
 *		FUNCTIONS				
 ************************************/

int32_t ClusterLow(char *filename)
{
	char *correctFilename = (char *) malloc(11);
	correctFilename = compare(filename); // Get the correctly formatted filename
		
	int i;
	for (i = 0 ; i < 16 ; i++)
	{
		char *dir_name = (char *) malloc(11);
		memset(dir_name, '\0', 11);
		memcpy(dir_name, dir[i].DIR_Name, 11);
		
		// Looking for a match in file image system
		// If matched, get the cluster corresponded to that filename
		// If no, return -1
		if (strncmp(dir_name, correctFilename, 11) == 0)
		{
			int cluster = dir[i].DIR_FirstClusterLow;
			free(correctFilename);
			free(dir_name);
			return cluster;
		}
	}
	free(correctFilename);
	return -1;
}

int32_t ClusterSize(int32_t cluster)
{
	int i;
	for (i = 0 ; i < 16 ; i++)
	{
		if (cluster ==  dir[i].DIR_FirstClusterLow)
		{
			int size = dir[i].DIR_FileSize;
			return size;
		}
	}
	return -1;
}

void open(char *filename)
{
	FileHandle = fopen(filename, "r");
	if (FileHandle == NULL || filename == NULL)
	{
		printf("Error: File system image not found.\n");
		return;
	}
	
	fseek(FileHandle, 3, SEEK_SET);
	fread(&BS_OEMName, 8, 1, FileHandle);
	
	fseek(FileHandle, 11, SEEK_SET);
	fread(&BPB_BytsPerSec, 2, 1, FileHandle);
	
	fseek(FileHandle, 13, SEEK_SET);
	fread(&BPB_SecPerClus, 1, 1, FileHandle);
	
	fseek(FileHandle, 14, SEEK_SET);
	fread(&BPB_RsvdSecCnt, 2, 1, FileHandle);
	
	fseek(FileHandle, 16, SEEK_SET);
	fread(&BPB_NumFATs, 1, 1, FileHandle);
	
	fseek(FileHandle, 17, SEEK_SET);
	fread(&BPB_RootEntCnt, 2, 1, FileHandle);
	
	fseek(FileHandle, 36, SEEK_SET);
	fread(&BPB_FATSz32, 4, 1, FileHandle);
	
	fseek(FileHandle, 44, SEEK_SET);
	fread(&BPB_RootClus, 4, 1, FileHandle);
	
	currentDirectory = BPB_RootClus;

	fseek(FileHandle, LBAToOffset(currentDirectory), SEEK_SET);
	fread(&dir[0], sizeof(struct DirectoryEntry), 16, FileHandle);
	
}

void info()
{
	printf("%25s %15s\n", "Decimal", "Hexadecimal");
	printf("%-15s %5d %15x\n", "BPB_BytsPerSec:", BPB_BytsPerSec, BPB_BytsPerSec);
	printf("%-15s %5d %15x\n", "BPB_SecPerClus:", BPB_SecPerClus, BPB_SecPerClus);
	printf("%-15s %5d %15x\n", "BPB_RsvdSecCnt:", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
	printf("%-15s %5d %15x\n", "BPB_NumFATs:", BPB_NumFATs, BPB_NumFATs);
	printf("%-15s %5d %15x\n", "BPB_FATSz32:", BPB_FATSz32, BPB_FATSz32);
}

void stat(char *filename)
{
	int cluster = ClusterLow(filename);
	int size = ClusterSize(cluster);
	if (size == -1)
	{
		printf("Error: File not found.\n");
		return;
	}
	printf("%-20s %5d\n", "File Size:", size);
	int i;
	for (i = 0 ; i < 16 ; i++)
	{
		if (cluster == dir[i].DIR_FirstClusterLow)
		{
			printf("%-20s %5d\n", "First Clus,ter Low:", cluster);
			printf("%-20s %5d\n", "DIR_ATTR:", dir[i].DIR_Attr);
			printf("%-20s %5d\n", "First Cluster High:", dir[i].DIR_FirstClusterHigh);
		}
	}
}

void get(char * filename)
{
	char *str = (char *) malloc(strlen(filename));
	strncpy(str, filename, strlen(filename));
	int cluster = ClusterLow(str);
	if (cluster == -1)
	{
		printf("Error: File not found.\n");
		return;
	}
	int size = ClusterSize(cluster);
	FILE *Destination = fopen(filename, "w+");
	fseek(FileHandle, LBAToOffset(cluster), SEEK_SET);
	unsigned char *ptr = (unsigned char *) malloc(size);
	fread(ptr, size, 1, FileHandle);
	fwrite(ptr, size, 1, Destination);
	fclose(Destination);
	free(ptr);
}

void cd(char *path)
{
	if (strcmp(path, "..") == 0)
	{
		int i;
		for (i = 0 ; i < 16 ; i++)
		{
			if (strncmp(dir[i].DIR_Name, "..", 2) == 0)
			{
				// currentDirectory = dir[i].DIR_FirstClusterLow + 2 because the sector
				// starts at 2 while the root directory's cluster # is 0, +2 so that
				// 'cd ..' is called, it changes to the correct root directory. 
				currentDirectory = dir[i].DIR_FirstClusterLow;
				if (currentDirectory == 0)
				{
					currentDirectory = dir[i].DIR_FirstClusterLow+2;
				}
				fseek(FileHandle, LBAToOffset(dir[i].DIR_FirstClusterLow), SEEK_SET);
				fread(&dir[0], sizeof(struct DirectoryEntry), 16, FileHandle);
				return;
			}
		}
	}
	int cluster = ClusterLow(path);
	// If the path passed in ClusterLow() does not exist, ClusterLow() returns -1.
	// cd() only changes directory if the path exists. Otherwise, it changes nothing.
	if (cluster != -1)
	{
		currentDirectory = cluster;
		fseek(FileHandle, LBAToOffset(cluster), SEEK_SET);
		fread(&dir[0], sizeof(struct DirectoryEntry), 16, FileHandle);
	}
}

void checkPath(char *path)
{
	char *temp = path;
	int count = 0;
	// Consider foo/foo1, the path that needs to cd() contains 2 sub-paths
	// this while() loop will count the number of '/' in the input path,
	// and the number of sub-paths equals the number of '/' + 1
	while ((temp = strchr(temp, '/')) != NULL)
	{
		count++;
		temp++;
	}
	char *file[count+1] = {};
	int i = 0;
	// If the path contains multiple sub-paths
	if (count != 0)
	{
		while (strcmp(path, "/") != 0 && strcmp(path, ".") != 0)
		{
			// basename() returns the last sub-path
			char *base = basename(path);
			file[i] = strdup(base);
			// dirname() returns the remaining path
			path = dirname(path);
			i++;
		}
		// Passing path to cd() starting from the last index, since cd()
		// changes parent directory before getting to its children.
		int j = count;
		while (j > -1)
		{
			cd(file[j]);
			free(file[j]);
			j--;
		}
	}
	// If the path only has one directory/file
	else
	{
		cd(path);
	}
}

void ls()
{
	fseek(FileHandle, LBAToOffset(currentDirectory), SEEK_SET);
	fread(&dir[0], sizeof(struct DirectoryEntry), 16, FileHandle);
	
	int i;
	for (i = 0 ; i < 16 ; i++)
	{
		// 0xe5 - The file has been used, but it has been deleted -> no show
		// 0x01 - Indicate that the file is read only
		// 0x10 - The entry describes a subdirectory
		// 0x20 - Indicate a hidden file. Such files can be displayed if it is really required.
		// Only show if attribute is 0x01, 0x10, or 0x20
		if ((dir[i].DIR_Name[0] != (char)0xe5) && (dir[i].DIR_Attr == 0x01 || 
			dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20))
		{
			char filename[12];
			strncpy(&filename[0], &dir[i].DIR_Name[0], 11);
			filename[12] = '\0';
			printf("%s\n", filename);
		}
	}
}

void read(char *filename, int position, int numOfBytes)
{
	int cluster = ClusterLow(filename);
	if (cluster == -1)
	{
		printf("Error: File not found.\n");
		return;
	}
	int offset = LBAToOffset(cluster);
	fseek(FileHandle, offset+position, SEEK_SET);
	unsigned char buffer[numOfBytes] = {};
	fread(&buffer[0], numOfBytes, 1, FileHandle);
	int i;
	for (i = 0 ; i < numOfBytes ; i++)
	{
		printf("%x ", buffer[i]);
	}
	printf("\n");
}

int main()
{
	char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
	
	while( 1 )
	{
		// Print out the mfs prompt
		printf ("mfs> ");
		
		// Read the command from the commandline.  The
		// maximum command that will be read is MAX_COMMAND_SIZE
		// This while command will wait here until the user
		// inputs something since fgets returns NULL when there
		// is no input
		while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

		/* Parse input */
		char *token[MAX_NUM_ARGUMENTS];
		
		int   token_count = 0;                                 
                                                           
		// Pointer to point to the token
		// parsed by strsep
		char *arg_ptr;                                         
                                                           
		char *working_str  = strdup( cmd_str );                

		// we are going to move the working_str pointer so
		// keep track of its original value so we can deallocate
		// the correct amount at the end
		char *working_root = working_str;
		
		// Tokenize the input stringswith whitespace used as the delimiter
		while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
			(token_count<MAX_NUM_ARGUMENTS))
		{
			token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
			if( strlen( token[token_count] ) == 0 )
			{
				token[token_count] = NULL;
			}
			token_count++;
		}
		
		// Now print the tokenized input as a debug check
		// \TODO Remove this code and replace with your FAT32 functionality
		/*
		int token_index  = 0;
		for( token_index = 0; token_index < token_count; token_index ++ ) 
		{
			printf("token[%d] = %s\n", token_index, token[token_index] );
		}
		*/
		// token[0] != NULL when the user gives valid input (not pressing enter)
		if (token[0] != NULL)
		{
			if (!strcmp(token[0], "open"))
			{
				if (FileHandle != NULL)
				{
					printf("Error: File system image already open.\n");
					continue;
				}
				else
				{
					open(token[1]);
				}
			}
			else if (!strcmp(token[0], "close"))
			{
				if (FileHandle == NULL)
				{
					printf("Error: File system not open.\n");
					continue;
				}
				fclose(FileHandle);
				FileHandle = NULL;
			}
			else if (!strcmp(token[0], "info"))
			{
				if (FileHandle == NULL)
				{
					printf("Error: File system image must be opened first.\n");
					continue;
				}
				info();
			}
			else if (!strcmp(token[0], "stat"))
			{
				if (FileHandle == NULL)
				{
					printf("Error: File system image must be opened first.\n");
					continue;
				}
				if (token[1] == NULL)
				{
					printf("Error: File not found.\n");
					continue;
				}
				stat(token[1]);
			}
			else if (!strcmp(token[0], "get"))
			{
				if (FileHandle == NULL)
				{
					printf("Error: File system image must be opened first.\n");
					continue;
				}
				if (token[1] == NULL)
				{
					printf("Error: File not found.\n");
					continue;
				}
				get(token[1]);
			}
			else if (!strcmp(token[0], "cd"))
			{
				if (FileHandle == NULL || token[1] == NULL)
				{
					printf("Error: File system image must be opened first.\n");
					continue;
				}
				checkPath(token[1]);
			}
			else if (!strcmp(token[0], "ls"))
			{
				if (FileHandle == NULL)
				{
					printf("Error: File system image must be opened first.\n");
					continue;
				}
				ls();
			}
			else if (!strcmp(token[0], "read"))
			{
				if (FileHandle == NULL)
				{
					printf("Error: File system image must be opened first.\n");
					continue;
				}
				if (token[1] == NULL || token[2] == NULL || token[3] == NULL)
				{
					printf("Not enough info to read\n");
					continue;
				}
				read(token[1], atoi(token[2]), atoi(token[3]));
			}
		}
		
		free( working_root );
	}
	
	return 0;
}
