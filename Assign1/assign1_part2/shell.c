/* SMP1: Simple Shell */

/* LIBRARY SECTION */
#include <ctype.h>              /* Character types                       */
#include <stdio.h>              /* Standard buffered input/output        */
#include <stdlib.h>             /* Standard library functions            */
#include <string.h>             /* String operations                     */
#include <sys/types.h>          /* Data types                            */
#include <sys/wait.h>           /* Declarations for waiting              */
#include <unistd.h>             /* Standard symbolic constants and types */
#include <sys/mman.h>
#include "smp1_tests.h"         /* Built-in test system                  */

/* DEFINE SECTION */
#define SHELL_BUFFER_SIZE 256   /* Size of the Shell input buffer        */
#define SHELL_MAX_ARGS 8        /* Maximum number of arguments parsed    */
#define MAXIMUM_REUSED_COMMAND 9 /* Maximum number of reused commands by !NN */
/* VARIABLE SECTION */
enum { STATE_SPACE, STATE_NON_SPACE };	/* Parser states */


int imthechild(const char *path_to_exec, char *const args[])
{
	return execvp(path_to_exec, args) ? -1 : 0; //use execvp to execute 
}
//Change imtheparent to return child_error_code for main function to know if child process failed or not
int imtheparent(pid_t child_pid, int run_in_background)
{
	int child_return_val, child_error_code;

	/* fork returned a positive pid so we are the parent */
	fprintf(stderr,
	        "  Parent says 'child process has been forked with pid=%d'\n",
	        child_pid);
	if (run_in_background) {
		fprintf(stderr,
		        "  Parent says 'run_in_background=1 ... so we're not waiting for the child'\n");
		return 0;
	}
	//wait(&child_return_val);
	waitpid(-1, &child_return_val, 0);
	/* Use the WEXITSTATUS to extract the status code from the return value */
	child_error_code = WEXITSTATUS(child_return_val);
	fprintf(stderr,
	        "  Parent says 'wait() returned so the child with pid=%d is finished.'\n",
	        child_pid);
	if (child_error_code != 0) {
		/* Error: Child process failed. Most likely a failed exec */
		fprintf(stderr,
		        "  Parent says 'Child process %d failed with code %d'\n",
		        child_pid, child_error_code);
	}
	return child_error_code;
}
int isNumber(char*str){
	int len = strlen(str);
	for(int i = 0; i < len ; ++i){
		if(!isdigit(str[i])) return 0;
	}
	return 1;
}

int global_var = 0;
// static int *glob_var;
/* MAIN PROCEDURE SECTION */
int main(int argc, char **argv)
{
	pid_t shell_pid, pid_from_fork;
	int n_read, i, exec_argc, parser_state, run_in_background, counter = 1;
	//int subshell_count = 1;
	/* buffer: The Shell's input buffer. */
	char buffer[SHELL_BUFFER_SIZE];
	/* exec_argv: Arguments passed to exec call including NULL terminator. */
	char *exec_argv[SHELL_MAX_ARGS + 1];
	

	char **argv_store[MAXIMUM_REUSED_COMMAND]; //to store previous argv commands
	for(int i = 0; i < MAXIMUM_REUSED_COMMAND; ++i){
		argv_store[i] = malloc((SHELL_MAX_ARGS + 1) * sizeof(char*));
	}
	/* Entrypoint for the testrunner program */
	if (argc > 1 && !strcmp(argv[1], "-test")) {
		return run_smp1_tests(argc - 1, argv + 1);
	}

	/* Allow the Shell prompt to display the pid of this process */
	while (1) {
		

	/* The Shell runs in an infinite loop, processing input. */

		shell_pid = getpid();
		fprintf(stdout, "Shell(pid=%d)%d> ", shell_pid, counter);
		fflush(stdout);

		/* Read a line of input. */
		if (fgets(buffer, SHELL_BUFFER_SIZE, stdin) == NULL)
			return EXIT_SUCCESS;
		n_read = strlen(buffer);

		run_in_background = n_read > 2 && buffer[n_read - 2] == '&';
		buffer[n_read - run_in_background - 1] = '\n';

		/* Parse the arguments: the first argument is the file or command *
		 * we want to run.                                                */

		parser_state = STATE_SPACE;
		for (exec_argc = 0, i = 0;
		     (buffer[i] != '\n') && (exec_argc < SHELL_MAX_ARGS); i++) {

			if (!isspace(buffer[i])) {
				if (parser_state == STATE_SPACE)
					exec_argv[exec_argc++] = &buffer[i];
				parser_state = STATE_NON_SPACE;
			} else {
				buffer[i] = '\0';
				parser_state = STATE_SPACE;
			}
		}

		/* run_in_background is 1 if the input line's last character *
		 * is an ampersand (indicating background execution).        */


		buffer[i] = '\0';	/* Terminate input, overwriting the '&' if it exists */

		/* If no command was given (empty line) the Shell just prints the prompt again */
		if (!exec_argc)
			continue;
		/* Terminate the list of exec parameters with NULL */
		exec_argv[exec_argc] = NULL;
		if(counter <= MAXIMUM_REUSED_COMMAND){
			//Store current arguments which may be reused by !NN command
			for(int i = 0; i < exec_argc; ++i){
				argv_store[counter - 1][i] = malloc(SHELL_BUFFER_SIZE * sizeof(char));
				strcpy(argv_store[counter - 1][i], exec_argv[i]);
			}
			//Set last argument tobe NULL
			argv_store[counter-1][exec_argc] = NULL;
		}
		/* If Shell runs 'exit' it exits the program. */
		if (!strcmp(exec_argv[0], "exit")) {
			printf("Exiting process %d\n", shell_pid);
			global_var--;
			return EXIT_SUCCESS;	/* End Shell program */

		} else if (!strcmp(exec_argv[0], "cd") && exec_argc > 1) {
		/* Running 'cd' changes the Shell's working directory. */
			/* Alternative: try chdir inside a forked child: if(fork() == 0) { */
			if (chdir(exec_argv[1]))
				 //Error: change directory failed 
				fprintf(stderr, "cd: failed to chdir %s\n", exec_argv[1]);	
			/* End alternative: exit(EXIT_SUCCESS);} */

		} else {
			char**exec_argv_dynamic_ptr = exec_argv;
			//Handle reuse command !NN
			if (exec_argv[0][0] == '!'){
				int nth_command = exec_argv[0][1] - '0';
				//Invalid command
				if(nth_command >= counter){
					fprintf(stderr, "Not valid\n");
					continue;
				}
				exec_argv_dynamic_ptr = argv_store[nth_command - 1];
			}
			//Handle sub command
			int isSubCommand  = 0;
			if(!strcmp(exec_argv_dynamic_ptr[0], "sub")){
				isSubCommand = 1;
					// printf("global_var=%d\n", global_var);

				if(global_var >= 2){
					fprintf(stderr, "Too deep!\n");
					continue;
				}	
			}
			/* Execute Commands */
			/* Try replacing 'fork()' with '0'.  What happens? */

			pid_from_fork = fork();
			if (pid_from_fork < 0) {
				/* Error: fork() failed.  Unlikely, but possible (e.g. OS *
				 * kernel runs out of memory or process descriptors).     */
				fprintf(stderr, "fork failed\n");
				continue;
			}
			if (pid_from_fork == 0) {
				
				if(isSubCommand){
					global_var++;
					counter = 1;
					continue;
				}
				return imthechild(exec_argv_dynamic_ptr[0], &exec_argv_dynamic_ptr[0]);
				/* Exit from main. */
			} else {
				/* Parent will continue around the loop. */
				int child_error_code = imtheparent(pid_from_fork, run_in_background);
				//If the child failed, do not increment counter
				if(child_error_code != 0){
					counter--;	
				}
			}
			counter++;	
		} /* end if */
		
	} /* end while loop */

	//Free dynamic pointers
	for(int i = 0; i < MAXIMUM_REUSED_COMMAND; ++i){
		argv_store[i] = malloc((SHELL_MAX_ARGS + 1) * sizeof(char*));
		for(int j = 0; j < SHELL_MAX_ARGS + 1; ++j){
			free(argv_store[i][j]);
		}
		free(argv_store[i]);
	}
	free(argv_store);

	return EXIT_SUCCESS;
} /* end main() */
