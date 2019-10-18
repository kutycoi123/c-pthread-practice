/* calc.c - Multithreaded calculator */

#include "calc.h"

pthread_t adderThread;
pthread_t degrouperThread;
pthread_t multiplierThread;
pthread_t readerThread;
pthread_t sentinelThread;


char buffer[BUF_SIZE];
int num_ops;
int isValidExpression; /*1 if expression is valid, 0 if invalid*/
int IS_CHECKING_VALID_EXP; /*1 if are only checking valid expression, 0 if actually calculating the expression*/

pthread_mutex_t lock_num_ops = PTHREAD_MUTEX_INITIALIZER; /*Mutex for num_ops*/

sem_t lock_buffer; //Semaphore for buffer
sem_t lock_valid_exp; //Semaphore for isValidExpression


/* Utiltity functions provided for your convenience */

/* int2string converts an integer into a string and writes it in the
   passed char array s, which should be of reasonable size (e.g., 20
   characters).  */
char *int2string(int i, char *s)
{
    sprintf(s, "%d", i);
    return s;
}

/* string2int just calls atoi() */
int string2int(const char *s)
{
    return atoi(s);
}

/* isNumeric just calls isdigit() */
int isNumeric(char c)
{
    return isdigit(c);
}

/* End utility functions */


void printErrorAndExit(char *msg)
{
    msg = msg ? msg : "An unspecified error occured!";
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

int timeToFinish()
{
    /* be careful: timeToFinish() also accesses buffer */
    // printf("Buffer in time to finish = %s\n", buffer);
    return buffer[0] == '.';
}
/*Find first non-negative integer in buffer
  Return the index = index of last digit + 1
*/
int findInt(char* buffer, int len, int*result, int iter){
	int intFound = 0, //An integer is found
		res = -1; //Decimal value of found integer

	while(iter < len){
		int checkNumeric = isNumeric(buffer[iter]);
		if( checkNumeric && !intFound){
			intFound = 1;
			res = buffer[iter] - '0';
		}else if(checkNumeric){
			res = res * 10 + (buffer[iter] - '0');
		}else if(!checkNumeric && intFound){
			break;
		}
		iter++;
	}
	*result = res;
	if(!intFound){
		return -1;
	}
	return iter;
}

/*Perfome add operation for every expression in buffer
  by finding pattern (a+b) and modify buffer to be (c), c = a + b
  Return 1 if expressions in buffer are valid, 0 if any expression is invalid
 */
int add_op(char* buffer){
	int startOffset, remainderOffset; //(1*2) => startOffset is index of '1', remainderOffset is index of ')'
	int value1, value2; //(1+2) => 1 is value1, 2 is value2
	int bufferlen;
	int i;
	int isValid = 0;
	startOffset = remainderOffset = -1;
	value1 = value2 = -1;
	bufferlen = strlen(buffer);
	for (i = 0; i < bufferlen; i++) {
	    // do we have value1 already?  If not, is this a "naked" number?
	    if(isNumeric(buffer[i])){
		    if(value1 != -1){ // Already got value1
		    	remainderOffset = findInt(buffer, bufferlen, &value2, i); //Find value2
		    	if(!IS_CHECKING_VALID_EXP){ //If needs to calculate expression
		    		int res = value1 + value2; //Compute add operation	
			    	char resInString[BUF_SIZE];
			    	int2string(res, resInString); //Convert result into string 

				    strcat(resInString, buffer+remainderOffset);
				    strcpy(buffer+startOffset, resInString);
				    if(pthread_mutex_lock(&lock_num_ops) != 0)  //Lock num_ops
						printErrorAndExit("In adder: lock failed\n");
					
					num_ops++;
					if (pthread_mutex_unlock(&lock_num_ops) != 0) //Unlock num_ops
						printErrorAndExit("In adder: unlock failed\n");

					isValid = 1;
					//Then reset variables
				    value1 = value2 = -1;
				    startOffset = remainderOffset = -1;
				    bufferlen = strlen(buffer);
			    	i = -1; //Come back to the beginning so as not to miss any addition
			    	continue;
		    	}else{ // Only need to check if expression is valid
		    		isValid = 1;
		    		break;
		    	}
		    	
		    }else{ //Not have value1 yet, let's find it

		    	startOffset = i;

		    	i = findInt(buffer, bufferlen, &value1, i);
		    	// printf("value1 = %d\n", value1);
		    	if(i != -1 && buffer[i] != '+'){ //value1 must be followed by '+'
		    		startOffset = -1;
		    		value1 = value2 = -1;
		    	}
		    	//printf("value1 = %d\n", value1);
		    }
		}else if (value1 != -1){
			value1 = value2 = -1;
			startOffset = remainderOffset = -1;
		}
	}
	if(bufferlen == 0) //If buffer is empty, it's a valid expression
		return 1;
	return isValid;
}
/*Perfome multiply operation for every expression in buffer
  by finding pattern (a*b) and modify it to be (c), c = a * b
  Return 1 if expressions in buffer are valid, 0 if any expression is invalid
 */
int multiply_op(char* buffer){
	int startOffset, remainderOffset; //(1*2) => startOffset is index of '1, remainderOffset is  index of ')'
	int value1, value2; //(1*2) => 1 is value1, 2 is value2
	int bufferlen;
	int i;
	int isValid;

	startOffset = remainderOffset = -1;
	value1 = value2 = -1;
	isValid = 0;
	bufferlen = strlen(buffer);

	for (i = 0; i < bufferlen; i++) {
		    // same as adder, but v1*v2
		    if(isNumeric(buffer[i])){
				if(value1 != -1){
				   	remainderOffset = findInt(buffer, bufferlen, &value2, i);
				   	if(!IS_CHECKING_VALID_EXP){
					    int res = value1 * value2; //Compute add operation	
					    char resInString[BUF_SIZE];
				    	int2string(res, resInString); //Convert result into string 

				    	strcat(resInString, buffer+remainderOffset);
				    	strcpy(buffer+startOffset, resInString);
					    if(pthread_mutex_lock(&lock_num_ops) != 0){  //Lock the shared resources
							printErrorAndExit("In multiply_op: lock failed\n");
						}
						num_ops++;
						if(pthread_mutex_unlock(&lock_num_ops) != 0){
							printErrorAndExit("In multiply_op: unlock failed\n");
						}
						bufferlen = strlen(buffer);
				    	value1 = value2 = -1;
				    	startOffset = remainderOffset = -1;
				    	// printf("new buffer = %s", buffer);
				    	i = -1;
				    	continue;
				   	}else{
				   		isValid = 1;
				   		break;
				   	}
				   	
			    }else{ //Not have value1 yet
			    	startOffset = i;
			    	i = findInt(buffer, bufferlen, &value1, i);
			    	if(buffer[i] != '*'){ //value1 must be followed by '+'
			    		startOffset = -1;
			    		value1 =value2=-1;
			    	}
			    	// printf("value1 in mul = %d\n", value1);
			    }
			} else if (value1 != -1){
				value1 = value2 = -1;
				startOffset = remainderOffset = -1;
			}
		}
	if(bufferlen == 0)
		return 1;
	return isValid;
}
/*Perfome degroup operation for every expression in buffer
  by finding pattern (a) and modify it to be a
  Return 1 if expressions in buffer are valid, 0 if any expression is invalid
 */
int degroup_op(char*buffer){
	int bufferlen;
	int i;
	int isValid;

	isValid = 0;
	bufferlen = strlen(buffer);

	for (i = 0; i < bufferlen; i++) {
		    // printf("buffer = %s\n", buffer);
		    if(buffer[i] == '('){ //Check '('
		    	int start = i;
		    	i++;
		    	while(i < bufferlen && isNumeric(buffer[i])) i++; //Get integer behind '('
		    	if(i < bufferlen && buffer[i] == ')'){ //Check ')'
		    		if(!IS_CHECKING_VALID_EXP){
		    			int end = i;
			    		strcpy(buffer+end, buffer+end+1); //Remove ')'
			    		strcpy(buffer+start, buffer+start+1); //Remove '('
							
						if(pthread_mutex_lock(&lock_num_ops) != 0)  //Lock the shared resources
							printErrorAndExit("In degroup_op: lock failed\n");
						num_ops++;
						if(pthread_mutex_unlock(&lock_num_ops) != 0)
							printErrorAndExit("In degroup_op: unlock failed\n");
			    		i--;
						bufferlen-=2;
		    		}else{
						isValid = 1;
		    			break;
		    		}
		    		
		    	}
				else{
					i = start;
				}
		    }
		}
	if(bufferlen == 0)
		return 1;
	return isValid;
}
/* Looks for an addition symbol "+" surrounded by two numbers, e.g. "5+6"
   and, if found, adds the two numbers and replaces the addition subexpression 
   with the result ("(5+6)*8" becomes "(11)*8")--remember, you don't have
   to worry about associativity! */
void *adder(void *arg)
{

    while (1) {

		if(sem_wait(&lock_buffer) != 0){
			if(sem_post(&lock_buffer) != 0)
				printErrorAndExit("In adder: unlock failed\n");
			printErrorAndExit("In adder: lock failed\n");
		}
		if(timeToFinish()){
			if(sem_post(&lock_buffer) != 0)
				printErrorAndExit("In adder: unlock failed\n");
			return NULL;
		}


		if(!isValidExpression){
			if (sem_post(&lock_buffer) != 0)
				printErrorAndExit("In adder: unlock failed\n");
			return NULL;
		}

		add_op(buffer);

		if(sem_post(&lock_buffer) != 0)
			printErrorAndExit("In adder: unlock failed\n");

		sched_yield();
    }
}

/* Looks for a multiplication symbol "*" surrounded by two numbers, e.g.
   "5*6" and, if found, multiplies the two numbers and replaces the
   mulitplication subexpression with the result ("1+(5*6)+8" becomes
   "1+(30)+8"). */
void *multiplier(void *arg)
{


    while (1) {


		if(sem_wait(&lock_buffer) != 0){
			if(sem_post(&lock_buffer) != 0)
				printErrorAndExit("In multiplier: unlock failed\n");
			printErrorAndExit("In multiplier: lock failed\n");
		}
		if(timeToFinish()){
			if(sem_post(&lock_buffer) != 0)
				printErrorAndExit("In multiplier: unlock failed\n");
			return NULL;
		}


		if(!isValidExpression){
			if (sem_post(&lock_buffer) != 0)
				printErrorAndExit("In multiplier: unlock failed\n");
			return NULL;
		}

		multiply_op(buffer);

		if(sem_post(&lock_buffer) != 0)
			printErrorAndExit("In multiplier: unlock failed\n");

		sched_yield();

    }
}


/* Looks for a number immediately surrounded by parentheses [e.g.
   "(56)"] in the buffer and, if found, removes the parentheses leaving
   only the surrounded number. */
void *degrouper(void *arg)
{

    while (1) {

    	if(sem_wait(&lock_buffer) != 0){
			if(sem_post(&lock_buffer) != 0)
				printErrorAndExit("In degrouper: unlock failed\n");
			printErrorAndExit("In degrouper: lock failed\n");
		}
		if(timeToFinish()){
			if(sem_post(&lock_buffer) != 0)
				printErrorAndExit("In degrouper: unlock failed\n");
			return NULL;
		}


		if(!isValidExpression){
			if (sem_post(&lock_buffer) != 0)
				printErrorAndExit("In degrouper: unlock failed\n");
			return NULL;
		}

		degroup_op(buffer);

		if(sem_post(&lock_buffer) != 0)
			printErrorAndExit("In degrouper: unlock failed\n");

		sched_yield();
    }
}


/* sentinel waits for a number followed by a ; (e.g. "453;") to appear
   at the beginning of the buffer, indicating that the current
   expression has been fully reduced by the other threads and can now be
   output.  It then "dequeues" that expression (and trailing ;) so work can
   proceed on the next (if available). */
void *sentinel(void *arg)
{
    char numberBuffer[20];
    int bufferlen;
    int i;
    int isValid;
    
    while (1) {
    	isValid = 0;

		if(sem_wait(&lock_buffer) != 0){
			if(sem_post(&lock_buffer) != 0)
				printErrorAndExit("In sentinel: unlock failed\n");
			printErrorAndExit("In sentinel: lock failed\n");
		}
		if(timeToFinish()){
			if(sem_post(&lock_buffer) != 0)
				printErrorAndExit("In sentinel: unlock failed\n");
			return NULL;
		}

		
		/* storing this prevents having to recalculate it in the loop */
		bufferlen = strlen(buffer);

		for (i = 0; i < bufferlen; i++) {
		    if (buffer[i] == ';') {
				if (i == 0) {
					sem_post(&lock_buffer);
				    printErrorAndExit("Sentinel found empty expression!");
				} else {
				    /* null terminate the string */
				    numberBuffer[i] = '\0';
				    /* print out the number we've found */
				    fprintf(stdout, "%s\n", numberBuffer);
				    /* shift the remainder of the string to the left */
				    strcpy(buffer, &buffer[i + 1]);
				    //printf("Buffer processed: %s\n", buffer);
				    isValid = 1;
				    break;
				}
			    } else if (!isNumeric(buffer[i])) {
					break;
			    } else {
					numberBuffer[i] = buffer[i];
		    }
		}

		/*Below is code to check if expression is valid or not*/
		if(bufferlen == 0)
			isValid = 1;

		IS_CHECKING_VALID_EXP = 1;
		if(!(isValid || add_op(buffer) || multiply_op(buffer) || degroup_op(buffer))){
			if(sem_wait(&lock_valid_exp) != 0)
				printErrorAndExit("In sentinel: lock failed\n");
			isValidExpression = 0;
			if( sem_post(&lock_valid_exp) != 0)
				printErrorAndExit("In sentinel: unlock failed\n");

			if(sem_post(&lock_buffer) != 0)
				printErrorAndExit("In sentinel: unlock failed\n");
			fprintf(stdout, "No progress can be made\n");
			
			exit(EXIT_FAILURE);
		}
		IS_CHECKING_VALID_EXP = 0;

		if(sem_post(&lock_buffer) != 0)
				printErrorAndExit("In sentinel: unlock failed\n");

		sched_yield();
    }
}

/* reader reads in lines of input from stdin and writes them to the
   buffer */
void *reader(void *arg)
{
    while (1) {
		char tBuffer[100];
		int currentlen;
		int newlen;
		int free;
		if (sem_wait(&lock_buffer) != 0 )
			printErrorAndExit("In reader: lock failed\n");

		fgets(tBuffer, sizeof(tBuffer), stdin);

		/* Sychronization bugs in remainder of function need to be fixed */

		newlen = strlen(tBuffer);
		currentlen = strlen(buffer);

		/* if tBuffer comes back with a newline from fgets, remove it */
		if (tBuffer[newlen - 1] == '\n') {
		    /* shift null terminator left */
		    tBuffer[newlen - 1] = tBuffer[newlen];
		    newlen--;
		}

		/* -1 for null terminator, -1 for ; separator */
		free = sizeof(buffer) - currentlen - 2;

		while ( free < newlen) {
			// spinwaiting
			sched_yield();
		}

		
		/* we can add another expression now */
		strcat(buffer, tBuffer);
		strcat(buffer, ";");
		if (sem_post(&lock_buffer) != 0)
			printErrorAndExit("In reader: unlock failed\n");
		/* Stop when user enters '.' */
		if (tBuffer[0] == '.') {
		    return NULL;
		}

    }
}


/* Where it all begins */
int smp3_main(int argc, char **argv)
{
    void *arg = 0;		/* dummy value */
	isValidExpression = 1; 
	IS_CHECKING_VALID_EXP = 0;
    sem_init(&lock_buffer, 0, 1);
    sem_init(&lock_valid_exp, 0, 1);

    /* let's create our threads */
    if (pthread_create(&multiplierThread, NULL, multiplier, arg)
	|| pthread_create(&adderThread, NULL, adder, arg)
	|| pthread_create(&degrouperThread, NULL, degrouper, arg)
	|| pthread_create(&sentinelThread, NULL, sentinel, arg)
	|| pthread_create(&readerThread, NULL, reader, arg)) {
	printErrorAndExit("Failed trying to create threads");
    }

    /* you need to join one of these threads... but which one? */
    pthread_detach(multiplierThread);
    pthread_detach(adderThread);
    pthread_detach(degrouperThread);
    pthread_detach(readerThread);
    pthread_join(sentinelThread, NULL);

    sem_destroy(&lock_buffer);
    sem_destroy(&lock_valid_exp);
    /* everything is finished, print out the number of operations performed */
    fprintf(stdout, "Performed a total of %d operations\n", num_ops);
    return EXIT_SUCCESS;
}
