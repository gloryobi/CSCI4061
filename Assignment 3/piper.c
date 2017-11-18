/***********************************************************************************************

 CSci 4061 Fall 2017
 Assignment# 3: Piper program for executing pipe commands 

 Student name: Glory Obielodan   
 Student ID:   4964466   

 Student name: Taewoo Kang   
 Student ID:   5328817

 X500 id: obiel001, kangx766

 Operating system on which you tested your code: Linux
 CSELABS machine: <CSEL-KH1250-03>

 GROUP INSTRUCTION:  Please make only ONLY one  submission when working in a group.
***********************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>

#define DEBUG

#define MAX_INPUT_LINE_LENGTH 2048 // Maximum length of the input pipeline command
                                   // such as "ls -l | sort -d +4 | cat "
#define MAX_CMDS_NUM   8           // maximum number of commands in a pipe list
                                   // In the above pipeline we have 3 commands
#define MAX_CMD_LENGTH 4096         // A command has no more than 4098  characters

FILE *logfp;

int num_cmds = 0;
char *cmds[MAX_CMDS_NUM];
int cmd_pids[MAX_CMDS_NUM];
int cmd_status[MAX_CMDS_NUM]; 

static sigjmp_buf jmpbuf;		    // signal handler
int newPipe[2];						// pipelines used between the processes
int oldPipe[2];


/*******************************************************************************/
/*   The function parse_command_line will take a string such as
     ls -l | sort -d +4 | cat | wc
     given in the char array commandLine, and it will separate out each pipe
     command and store pointer to the command strings in array "cmds"
     For example:
     cmds[0]  will pooint to string "ls -l"
     cmds[1] will point to string "sort -d +4"
     cmds[2] will point to string "cat"
     cmds[3] will point to string "wc"

     This function will write to the LOGFILE above information.
*/
/*******************************************************************************/

int parse_command_line (char commandLine[MAX_INPUT_LINE_LENGTH], char* cmds[MAX_CMDS_NUM]){
	int i = 0;											// variable to keep track of current command
	cmds[i] = strtok(commandLine, "|");					// split command line with string tokenizer by delimiter "|"

	if(cmds[i] == NULL){								// check if the command line is empty
		printf("Command Line is empty\n");
	}

	while(cmds[i] != NULL){								// continue splitting through the remaining commands with tokenizer
		fprintf(logfp, "Command [%d] info: \"%s\"\n", i, cmds[i]);
		i++;
		cmds[i] = strtok(NULL, "|");
	}
	fprintf(logfp, "Number of commands from the input: %d\n\n", i);		// print to LOGFILE number of commands
	return i;
}

/*******************************************************************************/
/*  parse_command takes command such as  
    sort -d +4
    It parses a string such as above and puts command program name "sort" in
    argument array "cmd" and puts pointers to ll argument string to argvector
    It will return  argvector as follows
    command will be "sort"
    argvector[0] will be "sort"
    argvector[1] will be "-d"
    argvector[2] will be "+4"
/
/*******************************************************************************/

void parse_command(char input[MAX_CMD_LENGTH],
                   char command[MAX_CMD_LENGTH],
                   char *argvector[MAX_CMD_LENGTH]){
	int i = 0;
	argvector[i] = strtok(input, " ");					// use string tokenizer to split command into arguments

	while(argvector[i] != NULL) {						// seperate all arguments into argvector array 
		i++;
		argvector[i] = strtok(NULL, " ");
	}
	
	strcpy(command, argvector[0]);						// copy 0th index for command name
}


/*******************************************************************************/
/*  The function print_info will print to the LOGFILE information about all    */
/*  processes  currently executing in the pipeline                             */
/*  This printing should be enabled/disabled with a DEBUG flag                 */
/*******************************************************************************/


void print_info(char* cmds[MAX_CMDS_NUM],
		int cmd_pids[MAX_CMDS_NUM],
		int cmd_stat[MAX_CMDS_NUM],
		int num_cmds) {
	#ifdef DEBUG										// DEBUG enabler
		fprintf(logfp, "\n");
		for(int i = 0; i < num_cmds; i++){				// for each command print info to LOGFILE
			fprintf(logfp, "Command: %s, Command PID: %d, Command Stat: %d\n", cmds[i], cmd_pids[i], cmd_stat[i]);
		}
		fprintf(logfp, "Number of Commands: %d\n\n", num_cmds);		// print number of commands to LOGFILE
	#endif
}  


/*******************************************************************************/
/*     The create_command_process  function will create a child process        */
/*     for the i'th command                                                    */
/*     The list of all pipe commands in the array "cmds"                       */
/*     the argument cmd_pids contains PID of all preceding command             */
/*     processes in the pipleine.  This function will add at the               */
/*     i'th index the PID of the new child process.                            */
/*     Following ADDED on  10/27/2017                                          */
/*     This function  will  craete pipe object, execute fork call, and give   */
/*     appropriate directives to child process for pipe set up and             */
/*     command execution using exec call                                       */
/*******************************************************************************/


void create_command_process (char cmds[MAX_CMD_LENGTH],   // Command line to be processed
                     int cmd_pids[MAX_CMDS_NUM],          // PIDs of preceding pipeline processes
                                                          // Insert PID of new command processs
		             int i)                               // commmand line number being processed
{
	int child_pid;
	char command[MAX_CMD_LENGTH];							// will contain command name
	char *cmd_argvector[MAX_CMD_LENGTH];					// array for command arguments

	if(i < (num_cmds - 1)) {								// if there is a future command
		if (pipe(newPipe) == -1) {							// create a pipe and check for error
			perror("ERROR: Failed to create pipe\n");		// exit if error occurs
			exit(1);
		}
	}

	if ((child_pid = fork()) == -1) {						// create child process
		perror("ERROR: Could not fork\n");
		// print to LOGFILE if error occurs
		fprintf(logfp, "Terminating the pipeline with process id %d for command %d: %s", child_pid, i, cmds);
		exit(-1);											// exit
	}

	if (child_pid) {										// parent
		cmd_pids[i] = child_pid;							// store child process id in ith index of pids array
		
		// print to LOGFILE
		fprintf(logfp, "Creating process for command %d with process id %d...\n", i, child_pid);

		if(i > 0) {											// close oldPipe if previous command exists
			close(oldPipe[0]);
			close(oldPipe[1]);
		}
		if(i < (num_cmds - 1)) {							// update oldPipe if future command exists
			oldPipe[0] = newPipe[0];
			oldPipe[1] = newPipe[1];
		}
	}
	else {													// child process
		// print to LOGFILE
		fprintf(logfp, "Executing Command %d with process id %d\n", i, getpid()); 

		if(i > 0) {											// set process' input to previous command's output
			close(oldPipe[1]);
			dup2(oldPipe[0], 0);
			close(oldPipe[0]);
		}

		if(i < (num_cmds-1)) {								// set preocess' output to next command's input
			close(newPipe[0]);
			dup2(newPipe[1], 1);
			close(newPipe[1]);
		}
		{
			parse_command(cmds, command, cmd_argvector);	// parse through command to seperate arguments
			execvp(command, cmd_argvector);					// execute command
		}

	} 
}


/********************************************************************************/
/*   The function waitPipelineTermination waits for all of the pipeline         */
/*   processes to terminate.                                                    */
/********************************************************************************/

void waitPipelineTermination () {
	for(int i = 0; i < num_cmds; i++){						// go through each command process
		fprintf(logfp, "waiting...\n");						// print waiting message to LOGFILE
		waitpid(cmd_pids[i], &cmd_status[i], 0);			// wait for process to terminate
		fprintf(logfp, "Process id %d finished with exit status %d\n", cmd_pids[i], cmd_status[i]);		// print process info to LOGFILE
	}
}

/********************************************************************************/
/*  This is the signal handler function. It should be called on CNTRL-C signal  */
/*  if any pipeline of processes currently exists.  It will kill all processes  */
/*  in the pipeline, and the piper program will go back to the beginning of the */
/*  control loop, asking for the next pipe command input.                       */
/********************************************************************************/

void killPipeline( int signum ) {
	for(int i = 0; i < num_cmds; i++){						// go through all processes and kill them
		kill(cmd_pids[i], SIGKILL);
	}
	printf("\nAll pipeline processes have been killed\n");	// print to console
	fprintf(logfp, "\nAll pipeline processes have been killed\n\n");		// print to LOGFILE
	fprintf(logfp, "********************************************************************************\n\n");
	siglongjmp(jmpbuf, 1);
}

/********************************************************************************/

int main(int ac, char *av[]){
	int i,  pipcount;
	//check usage
	if (ac > 1){
		printf("\nIncorrect use of parameters\n");
		printf("USAGE: %s \n", av[0]);
		exit(1);
	}

	/* Set up signal handler for CNTRL-C to kill only the pipeline processes  */
	sigsetjmp(jmpbuf, 1);

	logfp = fopen("LOGFILE", "a");
	fprintf(logfp, "********************************************************************************\n\n");

	while (1) {
		signal(SIGINT, SIG_DFL ); 
		pipcount = 0;

		/*  Get input command file anme form the user */
		char pipeCommand[MAX_INPUT_LINE_LENGTH];

		fflush(stdout);
		printf("\nGive a list of pipe commands: ");
		gets(pipeCommand); 
		char* terminator = "quit";
		printf("You entered: %s\n\n", pipeCommand);
		if ( strcmp(pipeCommand, terminator) == 0  ) {
			fflush(logfp);
			fclose(logfp);
			printf("Goodbye!\n\n");
			exit(0);
		}  

		num_cmds = parse_command_line( pipeCommand, cmds);

		/*  SET UP SIGNAL HANDLER  TO HANDLE CNTRL-C                         */
		signal(SIGINT, killPipeline); 

		/*  num_cmds indicates the number of commands in the pipeline        */

		/* The following code will create a pipeline of processes, one for   */
		/* each command in the given pipe                                    */
		/* For example: for command "ls -l | grep ^d | wc -l "  it will      */
		/* create 3 processes; one to execute "ls -l", second for "grep ^d"  */
		/* and the third for executing "wc -l"                               */

		for(i=0;i<num_cmds;i++){
			/*  CREATE A NEW PROCCES EXECUTTE THE i'TH COMMAND    */
			/*  YOU WILL NEED TO CREATE A PIPE, AND CONNECT THIS NEW  */
			/*  PROCESS'S stdin AND stdout  TO APPROPRIATE PIPES    */  
			create_command_process (cmds[i], cmd_pids, i);
		}

		print_info(cmds, cmd_pids, cmd_status, num_cmds);				// print info to LOGFILE

		waitPipelineTermination();										// wait for processes to terminate

		print_info(cmds, cmd_pids, cmd_status, num_cmds);				// print info to LOGFILE
		
		fprintf(logfp, "********************************************************************************\n\n");

	}
	return 0;
} //end main

/*************************************************/