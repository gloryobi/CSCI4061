/***********************************************************************************************

 CSci 4061 Fall 2017
 Assignment #5: Inter-process Communication using TCP/IP

 Student name: Glory Obielodan   
 Student ID:   4964466   

 Student name: Taewoo Kang   
 Student ID:   5328817

 X500 id: obiel001, kangx766

 Operating system on which you tested your code: Linux
 CSELABS machine: <CSEL-KH1250-03>

 GROUP INSTRUCTION:  Please make only ONLY one submission when working in a group.
***********************************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 6789
#define BUFFER_SIZE 1024
#define NAME_SIZE 256
#define QUEUE_SIZE 2
#define NUMBER_OF_CATEGORIES 3					// number of categories

int catCount = 0;								// keep track of category count
char* catFiles[NUMBER_OF_CATEGORIES];			// array for category files
char* catNames[NUMBER_OF_CATEGORIES];			// array for category names

void* handleClientRequest(void* clientSocket) {		// function for server threads
	char request[BUFFER_SIZE]; 					// request from client
	int terminate = 0;
	FILE* files[NUMBER_OF_CATEGORIES]; 			// hold category files
	
	int* sock = (int *) clientSocket;			// socket value
	struct sockaddr_in* client_addr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
	socklen_t clientLength = sizeof(*client_addr);
	
	if (getsockname(*sock, (struct sockaddr*) client_addr, &clientLength) != 0) {
		printf("Error passing client info to server thread\n");					// error checking
		perror("Abort");
		exit(4);
	}
	
	for (int i = 0; i < catCount; i++) {										// open all category files
		if ((files[i] = fopen(catFiles[i], "r")) == NULL) {
			printf("Error opening file %s\n", catFiles[i]);						// error checking
			perror("Abort");
			exit(5);
		}
	}
	
	while (!terminate) {														// run thread until client ends session
		char* response = (char*) calloc(BUFFER_SIZE, sizeof(char));				// response to client
		
		if (recv(*sock, request, BUFFER_SIZE,0) < 0){							// receive client request
			printf("Error reading stream from socket\n");
			perror("Abort");
			close(*sock);
			exit(6);
		}
		
		if (!strcmp(request, "GET: QUOTE CAT: ANY\n")) {						// if client presses <ENTER>
			int randIndex = rand() % 3;											// generate a random number between 0 and 2
			sprintf(request, "GET: QUOTE CAT: %s\n", catNames[randIndex]);		// get a quote from random category
		}
		
		if (!strncmp(request, "GET: QUOTE CAT: ", 16)) {					// if client gives a category
			int categoryExists = 0;
			for (int i = 0; i < catCount; i++) {						
				char* str = (char*) malloc(NAME_SIZE*sizeof(char));
				strcpy(str, "GET: QUOTE CAT: ");
				strcat(str, catNames[i]);
				strcat(str, "\n");
				if(!strcmp(str, request)) {										// match available categories with requested category
					if(fgets(response, BUFFER_SIZE, files[i]) == NULL) {		// check if at end of file
						rewind(files[i]);										// if so, go back to the beginning of the file
						fgets(response, BUFFER_SIZE, files[i]);
					}
					char* quoter = (char*) malloc(NAME_SIZE*sizeof(char));
					fgets(quoter, BUFFER_SIZE, files[i]);
					strcat(response, quoter);									// add quoter to quote
					free(quoter);
					categoryExists = 1;
				}
				free(str);
			}
			if (!categoryExists)										// if requested category was not found
				sprintf(response, "Category does not exist\n");
		}
		else if (!strcmp(request, "GET: LIST\n")) {						// if client enters 'list'
			for (int i = 0; i < catCount; i++) {						// create of list of all categories as response
				strcat(response, catNames[i]);
				strcat(response, "\n");
			}
		}
		else if (!strcmp(request, "BYE\n")) {								// if client enters 'bye'
			terminate = 1;												// set terminate to 1 so while loop ends
		}
		else{															// if request is unrecognized
			sprintf(response, "Unable to execute client's request\n");
		}
		
		if (!terminate) {												// if client is still active
			if (send(*sock, response, sizeof(request),0) < 0) {			// send response from server to client
				printf("Error writing on stream socket");
				perror("Abort");
				close(*sock);
				exit(7);
			}
		}
		free(response);
		response = NULL;
	}
	
	for (int i = 0; i < catCount; i++) {	// conduct closings
		fclose(files[i]);
	}
	close(*sock);
}


int main(int argc, char *argv[]) {
	int sock;
	struct sockaddr_in server_addr;
	FILE* configFile;
	char* configName = (char*) malloc(NAME_SIZE * sizeof(char));
	char* line = NULL;
	size_t length = 0;
	
	if (argv[1])											// check for a passed in argument
		strcpy(configName, argv[1]);						// copy argument into configName
	else
		strcpy(configName, "config.txt");					// if no argument copy "config.txt" into configName
	
	if ((configFile = fopen(configName, "r")) == NULL){		// open config file
		perror("Error opening file");
	}
	
	while(getline(&line, &length, configFile) != -1) {			// parse through config file to get categories and respective files
		char* catName = (char*) malloc(NAME_SIZE*sizeof(char));
		char* catFileName = (char*) malloc(NAME_SIZE*sizeof(char));
		catNames[catCount] = (char*) malloc(NAME_SIZE*sizeof(char));
		catFiles[catCount] = (char*) malloc(NAME_SIZE*sizeof(char));
		catName = strtok(line, ": \n");						// seperate category and file name
		strcpy(catNames[catCount], catName);						// place category in category array
		catFileName = strtok(NULL, ": \n");
		strcpy(catFiles[catCount], catFileName);					// place category file name in it's array
		catCount++;						// increment catCount
	}
	if (line){
		free(line);
	}
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){			// create socket
		printf("%s: Unable to open socket\n",argv[0]);			// error checking
		perror("Abort");
		exit(1);
	}

	server_addr.sin_family = AF_INET;							// load the server info
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(SERVER_PORT);
	
	if (bind(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {		// bind
		printf("Error binding socket address\n");									// error checking
		perror("Abort");
		exit(2);
	}
	
	while (1) {							// run indefinitely
		listen(sock, QUEUE_SIZE);		// listen for client requests
		int* clientSock = (int*) malloc(sizeof(int));
		struct sockaddr_in* client_addr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
		socklen_t clientLength = sizeof(*client_addr);
		
		if ((*clientSock = accept(sock, (struct sockaddr*) client_addr, &clientLength)) < 0) {		// accept client request
			printf("Error accepting request\n");		// error checking
			perror("Abort");
			close(sock);
			close(*clientSock);
			exit(3);
		}
		pthread_t pid;
		pthread_create(&pid, NULL, handleClientRequest, (void*) clientSock);		// create a new server thread for client
	}
	
	close(sock);			// closings
	return 0;
}