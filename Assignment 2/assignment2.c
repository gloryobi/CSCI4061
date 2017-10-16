/***********************************************************************************************

 CSci 4061 Fall 2017
 Assignment# 2:   I/O Programming on UNIX/LINUX

 Student name: Glory Obielodan,   Taewoo Kang
 Student ID:   4964466,   5328817
 X500 id: obiel001, kangx766

 Operating system on which you tested your code: Linux
 CSELABS machine: <CSEL-KH1250-03>

 GROUP INSTRUCTION:  Please make only ONLY one  submission when working in a group.

***********************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#define NAMESIZE 256
#define BUFSIZE 256
#define TOKENSIZE 100

void largestFiles(char* dirpath, char* file[3], int size[3]){	// find largest files
	struct stat statbuf;
	
    DIR *dir;													// Define the directory variable dir

    if((dir = opendir(dirpath)) == NULL)						// check if directory can be opened
    {
        perror("Cannot Open Directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *direntry;									// directory entry structure direntry
    chdir(dirpath);												// Go into given directory

    while((direntry = readdir(dir))!= NULL )					// Read each directory item
    {
        lstat(direntry->d_name, &statbuf);
        
        if(!(S_ISDIR(statbuf.st_mode)))							// Check if direntry is not a directory
        {
            char *currentFile;
            currentFile =(char *)malloc(NAMESIZE * sizeof(char));

            realpath(direntry->d_name, currentFile);			// get the relative path for current file

			if(statbuf.st_size > size[0])						// check to see if current file is the biggest
			{
				for (int i = 2; i > 0; i-=1){					// shift accordingly if it is
					file[i] = file[i-1];
					size[i] = size[i-1];
				}
				file[0] = currentFile;
				size[0] = statbuf.st_size;
			}
			else if(statbuf.st_size > size[1])					// check to see if current file is the second biggest
			{
				file[2] = file[1];								// shift accordingly if it is
				file[1] = currentFile;
				size[2] = size[1];
				size[1] = statbuf.st_size;
			}
			else if(statbuf.st_size > size[2])					// check to see if current file is the third biggest
			{
				file[2] = currentFile;
				size[2] = statbuf.st_size;
			}
        }
        else													// current file is a directory				
        {
            if(strcmp(direntry->d_name, ".") == 0 || strcmp(direntry->d_name, "..") == 0)
            {
                // Ignore directories "." and ".."
            }
            else
            {
                realpath(direntry->d_name, dirpath);
                largestFiles(dirpath, file, size);				// recurse through subdirectories
            }
        }
    }
    chdir("..");												// go up one directory
}

void zeroLengthFiles(char* dirpath){							// find zero length files
	struct stat statbuf;
	
    DIR *dir;													// Define the directory variable dir

    if((dir = opendir(dirpath)) == NULL)						// check if directory can be opened
    {
        perror("Cannot Open Directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *direntry;									// directory entry structure direntry
    chdir(dirpath);												// Go into given directory
	
    while((direntry = readdir(dir))!= NULL )					// Read each directory item
    {
        lstat(direntry->d_name, &statbuf);
        
        if(!(S_ISDIR(statbuf.st_mode)))							// Check if direntry is not a directory
        {
			char *currentFile;
            currentFile =(char *)malloc(NAMESIZE * sizeof(char));

            realpath(direntry->d_name, currentFile);
			
            if(statbuf.st_size == 0)							// check if size of current file is 0
            {
                printf("\"%s\"\n", currentFile);
            }
        }
        else
        {
            if(strcmp(direntry->d_name, ".") == 0 || strcmp(direntry->d_name, "..") == 0)
            {
                // Ignore directories "." and ".."
            }
            else
            {
                realpath(direntry->d_name, dirpath);
                zeroLengthFiles(dirpath);						// recurse through subdirectories
            }
        }
    }
    chdir("..");												// go up one directory
}

void permissions(char* dirpath, int permission){				// find file permissions
	struct stat statbuf;
	int statchmod;												// variable to hold permissions of files
	
    DIR *dir;													// Define the directory variable dir

    if((dir = opendir(dirpath)) == NULL)						// check if directory can be opened
    {
        perror("Cannot Open Directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *direntry;									// directory entry structure direntry
    chdir(dirpath);												// Go into given directory
	
    while((direntry = readdir(dir))!= NULL )
    {
        lstat(direntry->d_name, &statbuf);
			
        if(!(S_ISDIR(statbuf.st_mode)))							// Check if direntry is not a directory
        {
			char *currentFile;
            currentFile =(char *)malloc(NAMESIZE * sizeof(char));

            realpath(direntry->d_name, currentFile);
			
			statchmod = statbuf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);			// Get the permission of the current item

			if(permission == statchmod){											// compare permission with given code
				printf("\"%s\"\n", currentFile);
			}
        }
        else
        {
            if(strcmp(direntry->d_name, ".") == 0 || strcmp(direntry->d_name, "..") == 0)
            {
                // Ignore directories "." and ".."
            }
            else
            {
                realpath(direntry->d_name, dirpath);
                permissions(dirpath, permission);				// recurse through subdirectories
            }
        }
    }
    chdir("..");												// go up one directory
}

int fileCopy(char* src, char* dest)								// copy files
{
    char cpbuffer[BUFSIZE];
    ssize_t count;
    mode_t perms;
    int fdin, fdout;

    if  ((fdin = open(src, O_RDONLY)) == -1) {					// open source file
        perror("Error opening the input file");
        return 1;
    }
 
    if  ((fdout = open(dest, (O_WRONLY | O_CREAT), perms)) == -1 ) {		// open destination file
        perror("Error creating the output file");
        return 2;
    }
   
    while ((count=read(fdin, cpbuffer, BUFSIZE)) > 0) {						// copy file
        if (write(fdout, cpbuffer, count) != count) { 
            perror("Error writing");
            return 3;
        }
    } 

    if (count == -1) {
        perror("Error reading the input file");						// error reading a file
        return 4;
    }

    close(fdin);
    close(fdout);
    return 0;
}

void copyFiles(char* origDir, char* backup)							// function to copy files
{
	struct stat statbuf;
    char *dest, *oldLink, *newLink;
    dest = (char *) malloc(NAMESIZE * sizeof(char));
    oldLink = (char *) malloc(NAMESIZE * sizeof(char));
    newLink = (char *) malloc(NAMESIZE * sizeof(char));

    DIR *dir;														// Define the directory variable dir
	
    if ((dir = opendir(origDir)) == NULL) {							// check if directory can be opened
        perror("Cannot Open Directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *direntry;										// directory entry structure direntry for original directory
    chdir(origDir);													// go to the original directory
	
    while ((direntry = readdir(dir)) != NULL )
    {
        lstat(direntry->d_name, &statbuf);
		
        if (S_ISREG(statbuf.st_mode)) {								// if file is a regular file
            char *currentFile;
            currentFile =(char *)malloc(NAMESIZE * sizeof(char));

            realpath(direntry->d_name, currentFile);

			strcpy(dest, backup);									// copy backup directory name into dest
            strcat(dest, "/");
            strcat(dest, direntry->d_name);							// add file name to dest string
			
            if ((fileCopy(currentFile, dest)) != 0) {				// copy file to backup directory
                perror("Copying Error");
                exit(EXIT_FAILURE);
            }
        } 
		else if (S_ISLNK(statbuf.st_mode)) {						// if file is a syslink
            realpath(direntry->d_name, oldLink);
			
            strcpy(newLink, backup);
            strcat(newLink, "/");
            strcat(newLink, direntry->d_name);
			
            symlink(oldLink, newLink);								// create a new link in backup directory
        } 
		else { 														// "file" is a directory
            if (strcmp(direntry->d_name, ".") == 0 || strcmp(direntry->d_name, "..") == 0) {
                // Ignore directories "." and ".."
            } 
			else {
                realpath(direntry->d_name, origDir);

                strcat(backup, "/");
                strcat(backup, direntry->d_name);

                mkdir(backup, 0755);								// make a new directory is backup directory
                copyFiles(origDir, backup);							// recurse through subdirectories
            }
		}
    }
	
    if ((chdir("..")) == 0) {										// go up a directory
        getcwd(origDir, 1024);
    }
    else  { 
		perror("");
    }
    strcat(backup, "/..");
}

void backupDirectory(char *dirpath){
    char *origDir, *oldBackup, *date;
	time_t clocktime;
	struct tm *tm;
    oldBackup =(char *)malloc(NAMESIZE * sizeof(char));
    date =(char *)malloc(NAMESIZE * sizeof(char));
	origDir =(char *)malloc(NAMESIZE * sizeof(char));

	strcpy(origDir, dirpath);										// store dirpath in holding variable

    strcat(dirpath, ".bak");										// add extension name to dirpath

    if(mkdir(dirpath, 0755) == 0){									// create backup directory		
		//do nothing
    }
	else {															// error
        if(errno == EEXIST){										// a backup directory already exists
            printf("Backup directory already exists.\n");
                
			clocktime = time(NULL);									// get current time for date suffix
			tm = localtime(&clocktime);
			strftime(date, NAMESIZE, "-%b-%d-%Y-%H-%M-%S", tm);
            
            strcpy(oldBackup, dirpath);								// add date suffix for old backup
            strcat(oldBackup, date);
            printf("Renaming old backup to \"%s\"\n", oldBackup);
                
            rename(dirpath, oldBackup);								// rename
            printf("Creating new backup directory\n");
            mkdir(dirpath, 0755);									// create a new backup directory
        } 
		else {
            printf("Error occurred while creating backup directory.\n");		//unknown error
        }
    }
	copyFiles(origDir, dirpath);									// copy files into new backup directory
	printf("Backup directory created successfully.\n");
}

int main(int argc, char *argv[])
{
	int choice = -1;
	char *input_dir_name, *dirpath, *chptr;
	struct stat statbuf;

	input_dir_name =(char *)malloc(NAMESIZE * sizeof(char));
	dirpath =(char *)malloc(NAMESIZE * sizeof(char));
	printf("SELECT THE FUNCTION YOU WANT TO EXECUTE:\n");
	printf("1. Find the 3 largest files in a directory\n");
	printf("2. List all zero length files in a directory\n");
	printf("3. Find all files with a specified permission (i.e. 777) in a directory\n");
	printf("4. Create a backup of a directory\n");
	printf("5. Exit\n");
	printf("\n");
	printf("ENTER YOUR CHOICE: ");
	scanf("%d", &choice);
	
	if(choice == 5){
		printf("\nEXIT\n");
		exit(1);
	}
	else if(!(choice > 0 && choice < 5)){				// check for invalid inputs
		printf("Invalid choice\n");
		exit(3);
	}
	
	printf("Enter a directory name in the current directory: ");
	scanf("%s", input_dir_name);
	/**********************************************************/
	/*Form a full path to the directory and check if it exists*/
	/**********************************************************/
	realpath(input_dir_name, dirpath);
	
	if(stat(dirpath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)){		// check directory exists
		// do nothing
	}
	else{
		perror("");			// directory does not exist
		exit(2);
	}

	if(choice == 1){
		printf("\nEXECUTING \"1. Find the 3 largest files in a directory\"\n");
		printf("*****************************************************************************\n");
		/********************************************************/
		/**************Function to perform choice 1**************/
		/********************************************************/
		char* file[3];
		int size[3];
		
		for(int i = 0; i < 3; i+=1){
			file[i] = 0;
			size[i] = 0;
		}
		
		largestFiles(dirpath, file, size);						// function to find largest files
		for(int i = 0; i < 3; i+=1){							// print out
			printf("\nFile %d: \"%s\"\n", i+1, file[i]);
			printf("Size: %d bytes\n", size[i]);
		}
		printf("*****************************************************************************\n");
	}

	else if(choice == 2){
		printf("\nEXECUTING \"2. List all zero length files in a directory\"\n");
		printf("*****************************************************************************\n");
		/********************************************************/
		/**************Function to perform choice 2**************/
		/********************************************************/
		printf("\nThe following files are of zero byte length:\n");
		zeroLengthFiles(dirpath);								// function to find zero length files
		printf("*****************************************************************************\n");
	}

	else if(choice == 3){
		int input;
		printf("\nEXECUTING \"3. Find all files with specified permission (i.e 777) in a directory\"\n");
		printf("*****************************************************************************\n");
		/********************************************************/
		/**************Function to perform choice 3**************/
		/********************************************************/
		printf("\nEnter a 3 Octal digit (0-7) permission: ");		// get permission code from user
		scanf("%o", &input);
		
		printf("\nThe following are the files with permission %o in the given directory:\n", input);
		permissions(dirpath, input);								// function to find permission files
		printf("*****************************************************************************\n");
	}

	else if(choice == 4){
		printf("\nEXECUTING \"4. Create a backup of a directory\"\n");
		printf("*****************************************************************************\n");
		/********************************************************/
		/**************Function to perform choice 4**************/
		/********************************************************/
		printf("\nCreating a backup directory for given directory.\n\n");
		backupDirectory(dirpath);									// function to make backup directory
		printf("*****************************************************************************\n");
	}

	free(input_dir_name);
	free(dirpath);
	return 0;
}
