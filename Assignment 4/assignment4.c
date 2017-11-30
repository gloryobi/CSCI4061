/***********************************************************************************************

 CSci 4061 Fall 2017
 Assignment# 4: Concurrent Programming in C using POSIX thread library 

 Student name: Glory Obielodan   
 Student ID:   4964466   

 Student name: Taewoo Kang   
 Student ID:   5328817

 X500 id: obiel001, kangx766

 Operating system on which you tested your code: Linux
 CSELABS machine: <CSEL-KH1250-03>

 GROUP INSTRUCTION:  Please make only ONLY one submission when working in a group.
***********************************************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define NAMESIZE 256
#define NUM_THREADS 1000					// max number of threads
#define DEBUG	

typedef struct ThreadData					// will be used to pass data between threads
{
	int parentSize;
	char * childName;
} ThreadData;

pthread_mutex_t newDirectoryLock, sizeUpdateLock;
int initialThread;

void *parseDirectory(void *input)										// thread function
{
	DIR *dir;
	struct dirent *dirEntry;
	struct stat fStatbuf;
	char *dirName, *item;
	int threadCount = 0;
	ThreadData *parent, currentData;
	pthread_t tid[NUM_THREADS];
	
	
	parent = (ThreadData *) input;										// make a ThreadData pointer from input
	dirName = (char *) malloc(NAMESIZE * sizeof(char));
	strcpy(dirName, parent->childName);									// copy current directory name into dirName
	
	if(initialThread) 
		initialThread = 0;
	else 
		pthread_mutex_unlock(&newDirectoryLock);						// end critical section for setting name of new thread's directory
	
	currentData.parentSize = 0;											// initilize currentData and entry Name
	currentData.childName = (char *) malloc(NAMESIZE * sizeof(char));
	item = (char *) malloc(NAMESIZE * sizeof(char));	

	if( (dir = opendir(dirName)) == NULL)								// open current directory
	{
		perror("");
		exit(1);
	}
	
	while( (dirEntry = readdir(dir)) != NULL)							// parse through directory contents
	{
		if((strcmp(dirEntry->d_name, ".") == 0) ||
			(strcmp(dirEntry->d_name, "..") == 0))	
			continue;													// skip current directory and its parent

		strcpy(item, dirName);										
		strcat(item, "/");
		strcat(item, dirEntry->d_name);									// make a fullpath for the current item

		if((lstat(item, &fStatbuf)) == -1)								// check for error in lstat creation
		{
			perror("");
		}
		
		if(S_ISREG(fStatbuf.st_mode))									// if current item is a regular file
		{
			currentData.parentSize += fStatbuf.st_size;					// add size of item to parentSize
		}
		else if(S_ISDIR(fStatbuf.st_mode))								// if item is a directory
		{
			pthread_mutex_lock(&newDirectoryLock);						// create a critical section for setting name of new thread's directory
			strcpy(currentData.childName, item);						
			pthread_create(&tid[threadCount], NULL, parseDirectory, ((void *) &currentData));	// create a new thread for the subdirectory
			threadCount++;												// increment threadCount accordingly
		}
	}

	for(int i = 0; i < threadCount; i++)								// wait for child threads to complete execution
	{
		pthread_join(tid[i], NULL);
	}
	
	#ifdef DEBUG
		printf("DEBUG: %s/ %d\n", dirName, currentData.parentSize);		// output size of current directory
	#endif
	
	pthread_mutex_lock(&sizeUpdateLock);								// create critical section to update parent directory size
	parent->parentSize += currentData.parentSize;						// add to parent directory size
	pthread_mutex_unlock(&sizeUpdateLock);								// end critical section
	
	free(dirName);
	free(currentData.childName);
	free(item);
	pthread_exit(NULL);													// exit thread
}

int main(int argc, char *argv[])
{
	char *inputDir, *dirPath;
    struct stat statbuf;
	pthread_t tid;
	ThreadData dirInfo;

	pthread_mutex_init(&newDirectoryLock, NULL);						// initializations
	pthread_mutex_init(&sizeUpdateLock, NULL);
	inputDir = (char *) malloc(NAMESIZE * sizeof(char));
	dirPath = (char *) malloc(NAMESIZE * sizeof(char));
	
	printf("\n");
	printf("Enter a directory name: ");		// get input directory from user
	scanf("%s", inputDir);
	printf("\n");
	
	dirPath = getcwd(dirPath, NAMESIZE);
	strcat(dirPath, "/");
	strcat(dirPath, inputDir);											// create a fullpath for input directory
    
	
	if((lstat(dirPath, &statbuf)) == -1)								// check for error in lstat creation
	{
		perror("");
	}

	if(!(S_ISDIR(statbuf.st_mode))){									// check if input directory exists
		printf("The given directory does not exist\n");
		exit(1);
	}
	
	dirInfo.parentSize = 0;												// initialize dirInfo
	dirInfo.childName = (char *) malloc(NAMESIZE * sizeof(char));
	strcpy(dirInfo.childName, inputDir);								// set childName to inputDir
	initialThread = 1;													// to ensure there is no overlap problem when setting name new thread's directory
	
	pthread_create(&tid, NULL, parseDirectory, ((void *) &dirInfo));	// create the initial thread
	pthread_join(tid, NULL);											// wait for completion of all child threads
	printf("\nTotal Size: %d\n\n", dirInfo.parentSize);					// print total size of directory
	
	free(inputDir);
	free(dirPath);
	return 0;
}