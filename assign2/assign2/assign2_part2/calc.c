/* calc.c - Multithreaded calculator */

#include "calc.h"

pthread_t adderThread;
pthread_t degrouperThread;
pthread_t multiplierThread;
pthread_t readerThread;
pthread_t sentinelThread;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_1 = PTHREAD_MUTEX_INITIALIZER;
char buffer[BUF_SIZE];
int num_ops;


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
			res = res * 10 + buffer[iter] - '0';
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
/* Looks for an addition symbol "+" surrounded by two numbers, e.g. "5+6"
   and, if found, adds the two numbers and replaces the addition subexpression 
   with the result ("(5+6)*8" becomes "(11)*8")--remember, you don't have
   to worry about associativity! */
void *adder(void *arg)
{
    int bufferlen;
    int value1, value2;
    int startOffset, remainderOffset;
    int i;

    //return NULL; /* remove this line */

    while (1) {
		startOffset = remainderOffset = -1;
		value1 = value2 = -1;

		
		if(pthread_mutex_lock(&lock) != 0){  //Lock the shared resources
			printErrorAndExit("In adder: lock failed\n");
		}

		if (timeToFinish()) {
			pthread_mutex_unlock(&lock);
		    return NULL;
		}
		/* storing this prevents having to recalculate it in the loop */
		bufferlen = strlen(buffer);
		for (i = 0; i < bufferlen; i++) {
		    // do we have value1 already?  If not, is this a "naked" number?
		    if(isNumeric(buffer[i])){
		    	//printf("Here1\n");
			    if(value1 != -1){
			    	remainderOffset = findInt(buffer, bufferlen, &value2, i);

			    	int res = value1 + value2; //Compute add operation	
			    	char resInString[BUF_SIZE];
			    	int2string(res, resInString); //Convert result into string 
			    	int reslen = strlen(resInString);  

				    strcat(resInString, buffer+remainderOffset);
				    strcpy(buffer+startOffset, resInString);
				    // printf("value2 = %d\n", value2);
			    	// printf("Buffer after addition = %s\n", buffer);
				    if(pthread_mutex_lock(&lock_1) != 0){  //Lock the shared resources
						printErrorAndExit("In adder: lock ops failed\n");
					}
					num_ops++;
					pthread_mutex_unlock(&lock_1);
				    value1 = value2 = -1;
				    startOffset = remainderOffset = -1;
				    bufferlen = strlen(buffer);
			    	i = startOffset + reslen;
			    	continue;
			    }else{ //Not have value1 yet

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
				startOffset = -1;
			}
		    // if we do, is the next character after it a '+'?
		    // if so, is the next one a "naked" number?

		    // once we have value1, value2 and start and end offsets of the
		    // expression in buffer, replace it with v1+v2
		}
		pthread_mutex_unlock(&lock); //Release the shared resource
		// something missing?
		sched_yield();
    }
}

/* Looks for a multiplication symbol "*" surrounded by two numbers, e.g.
   "5*6" and, if found, multiplies the two numbers and replaces the
   mulitplication subexpression with the result ("1+(5*6)+8" becomes
   "1+(30)+8"). */
void *multiplier(void *arg)
{
    int bufferlen;
    int value1, value2;
    int startOffset, remainderOffset;
    int i;

    //return NULL; /* remove this line */

    while (1) {
		startOffset = remainderOffset = -1;
		value1 = value2 = -1;

		

		if (pthread_mutex_lock(&lock) != 0)
			printErrorAndExit("In multiplier: locking failed\n");

		if (timeToFinish()) {
			pthread_mutex_unlock(&lock);
		    return NULL;
		}
		/* storing this prevents having to recalculate it in the loop */
		bufferlen = strlen(buffer);
		//printf("Befter degrouping: %s\n", buffer);
		for (i = 0; i < bufferlen; i++) {
		    // same as adder, but v1*v2
		    if(isNumeric(buffer[i])){
			    	//printf("Here1\n");
				if(value1 != -1){
				   	remainderOffset = findInt(buffer, bufferlen, &value2, i);
				   	// printf("value2 = %d\n", value2);
				    int res = value1 * value2; //Compute add operation	
				    char resInString[BUF_SIZE];
			    	int2string(res, resInString); //Convert result into string 
			    	int reslen = strlen(resInString);  

			    	strcat(resInString, buffer+remainderOffset);
			    	strcpy(buffer+startOffset, resInString);

				    if(pthread_mutex_lock(&lock_1) != 0){  //Lock the shared resources
						printErrorAndExit("In adder: lock ops failed\n");
					}
					num_ops++;
					pthread_mutex_unlock(&lock_1);
					bufferlen = strlen(buffer);
			    	value1 = value2 = -1;
			    	startOffset = remainderOffset = -1;
			    	// printf("new buffer = %s", buffer);
			    	i = startOffset + reslen;
			    	continue;
			    }else{ //Not have value1 yet

			    	startOffset = i;

			    	i = findInt(buffer, bufferlen, &value1, i);
			    	if(buffer[i] != '*'){ //value1 must be followed by '+'
			    		startOffset = -1;
			    		value1 = -1;
			    	}
			    	// printf("value1 = %d\n", value1);
			    }
			} else if (value1 != -1){
				value1 = value2 = -1;
				startOffset = remainderOffset = -1;
			}
		}

		//printf("After degrouping: %s\n", buffer);
		pthread_mutex_unlock(&lock);
		sched_yield();
	// something missing?
    }
}


/* Looks for a number immediately surrounded by parentheses [e.g.
   "(56)"] in the buffer and, if found, removes the parentheses leaving
   only the surrounded number. */
void *degrouper(void *arg)
{
    int bufferlen;
    int i;

    //return NULL; /* remove this line */

    while (1) {

		
		if (pthread_mutex_lock(&lock) != 0)
			printErrorAndExit("In degrouper: locking failed\n");

		if (timeToFinish()) {
			pthread_mutex_unlock(&lock);
		    return NULL;
		}
		/* storing this prevents having to recalculate it in the loop */

		bufferlen = strlen(buffer);
		//printf("Befter degrouping: %s\n", buffer);
		for (i = 0; i < bufferlen; i++) {
		    // check for '(' followed by a naked number followed by ')'
		    // remove ')' by shifting the tail end of the expression
		    // remove '(' by shifting the beginning of the expression
		    // printf("buffer = %s\n", buffer);
		    if(buffer[i] == '('){
		    	int start = i;
		    	i++;
		    	while(i < bufferlen && isNumeric(buffer[i])) i++;
		    	if(i < bufferlen && buffer[i] == ')'){
		    		int end = i;
		    		strcpy(buffer+end, buffer+end+1);
		    		strcpy(buffer+start, buffer+start+1);
						
					if(pthread_mutex_lock(&lock_1) != 0){  //Lock the shared resources
						printErrorAndExit("In adder: lock ops failed\n");
					}
					num_ops++;
					pthread_mutex_unlock(&lock_1);

		    		i--;
					bufferlen-=2;
		    	}
				else{
					i = start;
				}
		    }
		}
		pthread_mutex_unlock(&lock);
		//printf("After degrouping: %s\n", buffer);
	// something missing?
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

    //return NULL; /* remove this line */

    while (1) {

		
		if (pthread_mutex_lock(&lock) != 0 )
			printErrorAndExit("In sentinel: locking failed\n");

		if (timeToFinish()) {
			pthread_mutex_unlock(&lock);
		    return NULL;
		}
		// printf("Buffer = %s\n", buffer);
		
		/* storing this prevents having to recalculate it in the loop */
		bufferlen = strlen(buffer);
		// printf("In sentinel: buffer = %s\n", buffer);

		for (i = 0; i < bufferlen; i++) {
		    if (buffer[i] == ';') {
				if (i == 0) {
					pthread_mutex_unlock(&lock);
				    printErrorAndExit("Sentinel found empty expression!");
				} else {
				    /* null terminate the string */
				    numberBuffer[i] = '\0';
				    /* print out the number we've found */
				    fprintf(stdout, "%s\n", numberBuffer);
				    /* shift the remainder of the string to the left */
				    strcpy(buffer, &buffer[i + 1]);
				    //printf("Buffer processed: %s\n", buffer);
				    break;
				}
			    } else if (!isNumeric(buffer[i])) {
					break;
			    } else {
					numberBuffer[i] = buffer[i];
		    }
		}
		pthread_mutex_unlock(&lock);

	// something missing?
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

		if (pthread_mutex_lock(&lock) != 0 )
			printErrorAndExit("In sentinel: locking failed\n");
		/* we can add another expression now */
		strcat(buffer, tBuffer);
		strcat(buffer, ";");
		// printf("%s\n", buffer);
		pthread_mutex_unlock(&lock);
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
    // pthread_detach(sentinelThread);
    // pthread_join(readerThread, NULL);

    /* everything is finished, print out the number of operations performed */
    fprintf(stdout, "Performed a total of %d operations\n", num_ops);
    return EXIT_SUCCESS;
}
