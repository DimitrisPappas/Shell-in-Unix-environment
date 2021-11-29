/*
Δημήτρης Παππάς 
ΑΕΜ: 8391
January 2018-19
OS Project Creating my own Shell in Unix Environment
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <readline/readline.h> 
#include <readline/history.h> 

#define MAXCHAR 512		// Max number of characters in a command line
#define clear() printf("\033[H\033[J")		// Clearing the shell using escape sequences 


// Functions
void init_shell();
void openHelp();
void printDir();
int cd(char* path);
int getInput(char* input);
void parseSpace(char* input, char** parsed);
void parseSymbols(int* nc, int* ns, char* input, char** commands, char* symbols);
int check(char* input);
void executeCommand(char** parsed);
void executeMultiCommands(int numberOfCommands, int numSymbols, char** commands, char* symbols);
int parsePipe(char* input, char** pipeArgs);
void executePipeCommands(char** parsePipeRead, char** parsePipeWrite);
int parseRedirection(char* input, char** redirArgs);
void executeRedirection(char** parseRedir, char** redirArgs);



// Main 
int main(int argc, char *argv[]){
	char input[MAXCHAR];			// input inserted from terminal
	char symbols[MAXCHAR-1];		// symbols ";" or "&"
	char *parsed[MAXCHAR];			// contains the word{s} of a command
	char *commands[MAXCHAR];		// contains the command of a line input
	char *pipeArgs[MAXCHAR];		// contains piping commands
	char *parsePipeRead[MAXCHAR];	// contains the words of Read command in pipe proccess
	char *parsePipeWrite[MAXCHAR];	// contains the words of Write command in pipe proccess
	char *redirArgs[MAXCHAR];		// contains redirection commands
	char *parseRedir[MAXCHAR];		// contains the words of the redirected command in redirection proccess
	int checkFlag = -1;				// -1: error, 0: simple command, 1: multiple commands
	int pipeFlag = 0;				// 0: pipe command not found, 1: pipe command is found
	int redirFlag = 0;				// 0: redirection command not found, 1: redirection command is found
	int nc, ns;						// number of commands, number of symbols
	char *filename;					// batchfile
	FILE *fp;						// file pointer
	long int size = 0;				// size of file
	char line[MAXCHAR];				// line of file
	int i;

	init_shell();
	
	// 		******		File management		******
	if (argv[1] != NULL){
		filename = argv[1];			// gets filename as argument
		fp = fopen(filename, "r");
		
		if (fp == NULL){
			printf("Error! File %s could not open\n", filename);
			exit(-1);
		}else{
			fseek (fp, 0, SEEK_END);
			size = ftell(fp);
			if (size == 0) {
				printf("File %s is empty\n", filename);
				exit(-1);
			}
		}
		
		// setting the file pointer in the start of the file
		fseek(fp, 0, SEEK_SET);
				
		while (fgets(line, MAXCHAR*sizeof(char), fp)){
			for (i=0;i<MAXCHAR;i++){
				if (line[i] == '\0')
					break;
				
				if (line[i] == '\n' && i != 0)
					line[i] = ' ';
			}
			//printf("%c\n", line[0]);
			if (line[0] == '\n')
				continue;
				
			strcpy(input, line);
			
			// check for multiple commands
			checkFlag = check(input);
			
			// check for pipes 
			pipeFlag = parsePipe(input, pipeArgs);
			//printf("pipeFlag = %d\n", pipeFlag);
			
			// check for redirection
			redirFlag = parseRedirection(input, redirArgs);
			//printf("redirFlag = %d\n", redirFlag);		
			
			if (checkFlag == 0){		// single command
				// parsing
				parseSpace(input, parsed);
				if (strcmp(input, "quit") == 0){
					printf("\nGoodbye!\nDo NOT come back (>_<)\n");
					exit(0);
				}else if (strcmp(parsed[0], "cd") == 0)
					// check for "cd" command
					cd(parsed[1]);
				else{
					if (pipeFlag == 0){		// if not pipe
						if (redirFlag == 1){	// if redirection
							// parse the redirected command
							parseSpace(redirArgs[0], parseRedir);
							// execute redirection 
							executeRedirection(parseRedir, redirArgs);
						}else{	// simple command
							// execute command
							executeCommand(parsed);
						}
						
					}else if (pipeFlag == 1){  
						// parse the read and write commands of the pipe proccess
						parseSpace(pipeArgs[0], parsePipeRead); 
						parseSpace(pipeArgs[1], parsePipeWrite);
						// execute pipe command
						executePipeCommands(parsePipeRead, parsePipeWrite);
					}
				}		
			}else if (checkFlag == 1){		// multiple commands
				parseSymbols(&nc, &ns, input, commands, symbols);
				//printf("%d + %d\n", nc, ns);
				executeMultiCommands(nc, ns, commands, symbols);
			}else{
				printf("No commands given\n");		
			}							
    	}
    	
		fclose(fp);
	}
		
	
	// 		******		Terminal Input management		******
	while(1){
		if (getInput(input))
			continue;
			
		// check for multiple commands
		checkFlag = check(input);
		
		// check for pipes 
		pipeFlag = parsePipe(input, pipeArgs);
		//printf("pipeFlag = %d\n", pipeFlag);
		
		// check for redirection
		redirFlag = parseRedirection(input, redirArgs);
		//printf("redirFlag = %d\n", redirFlag);	
			
		if (checkFlag == 0){		// single command
			// parsing
			parseSpace(input, parsed);
			if (strcmp(input, "quit") == 0){
				printf("\nGoodbye!\nDo NOT come back (>_<)\n");
				exit(0);
			}else if (strcmp(parsed[0], "cd") == 0)
				// check for "cd" command
				cd(parsed[1]);
			else{
				if (pipeFlag == 0){		// if not pipe
					if (redirFlag == 1){	// if redirection
						// parse the redirected command
						parseSpace(redirArgs[0], parseRedir);
						// execute redirection 
						executeRedirection(parseRedir, redirArgs);
					}else{	// simple command
						// execute command
						executeCommand(parsed);
					}
					
				}else if (pipeFlag == 1){  
					// parse the read and write commands of the pipe proccess
					parseSpace(pipeArgs[0], parsePipeRead); 
					parseSpace(pipeArgs[1], parsePipeWrite);
					// execute pipe command
					executePipeCommands(parsePipeRead, parsePipeWrite);
				}
			}		
		}else if (checkFlag == 1){		// multiple commands
			parseSymbols(&nc, &ns, input, commands, symbols);
			//printf("%d + %d\n", nc, ns);
			executeMultiCommands(nc, ns, commands, symbols);
		}else{
			printf("No commands given\n");		
		}	
	}
	
	return 0;
}


// Greeting shell during startup 
void init_shell(){ 
	clear(); 
	printf("\n\n\n******************************************"); 
	printf("\n\n\n\t****  D-SHELL  ****");  
	printf("\n\n******************************************"); 
	char* username = getenv("USER");		// searches for the environment string pointed to by "USER" and returns the associated value
	printf("\n\n\nUSER is: @%s", username); 
	printf("\n"); 
	sleep(1); 
	return;
}


// Help command list builtin 
void openHelp(){ 
	// 'puts()' writes a string to stdout
	puts("\n*** Welcome to d-Shell ***"
		"\nThis is an Early Access Shell\n"			
		"\nList of Commands supported:"
		"\n>cd"
		"\n>ls"
		"\n>exit"
		"\n>all other general commands available in UNIX shell"
		"\n>pipe handling"
		"\n>improper space handling\n"); 

	return; 
}


// Print Current Directory
void printDir(){
	char cwd[1024];			// current directory
	getcwd(cwd, sizeof(cwd));
	printf("\nDirectory: %s\n", cwd);
	return;
}

int cd(char* path){
	printf("%s\n", path);
	return chdir(path);
}


// Getter for Input command
int getInput(char* input){
	char *buff;
	
	buff = readline("Pappas_8391> ");		//  Adds a prompt and Read line from the terminal after the prompt
	if (strlen(buff) != 0){
		add_history(buff); 
		strcpy(input, buff);
		return 0;			
	}else{
		return 1;
	} 
}


// Parsing input line into words
void parseSpace(char* input, char** parsed){ 
	int i; 

	for (i = 0; i < MAXCHAR; i++) { 
		parsed[i] = strsep(&input, " "); 

		if (parsed[i] == NULL) 
			break; 
		if (strlen(parsed[i]) == 0) 
			i--; 
	} 
	return;
} 


// Parsing input line into commands
void parseSymbols(int* nc, int* ns, char* input, char** commands, char* symbols){
	int i;
	int numberOfCommands=0, numSymbols=0;
	
	// searching for ";" and "&"
	for (i=0;i<MAXCHAR;i++){
		if (input == NULL) 
			break;
			
		if (input[i] == ';'){
			symbols[numSymbols] = ';';
			numSymbols++;
		}else if (input[i] == '&'){
			symbols[numSymbols] = '&';
			numSymbols++;		
		}else if (input[i] == '\0'){
			break;
		}
	}
	
	for (i=0;i<numSymbols;i++){
		//printf("symbol[%d]: %c\n", i, symbols[i]);
	}
	
	// input is seperated into commands using ";" and "&" as seperator 
	for (i=0;i<MAXCHAR;i++){
		commands[i] = strsep(&input, ";&");		// each cell contains a command of the input line
		numberOfCommands++;
		if (commands[i] == NULL) 
			break; 	
		if (strlen(commands[i]) == 0)
			i--;
			
		//printf("command[%d]: %s\n", i, commands[i]);	
	}	
	*nc = numberOfCommands - 1;
	*ns = numSymbols;
	
	return;
}


// check if there are multiple commands
int check(char* input){
	if (strchr(input, ';') != NULL || strchr(input, '&') != NULL){
		return 1;
	}
	return 0;
}


// Function where the system command is executed
void executeCommand(char** parsed){
	pid_t pid = fork();		// create a child to execute the command
	if (pid < 0){
		printf("Error! Failed to fork child :(\n");
		return;
	}else if (pid == 0){	// child
		//printf("Child\n");
		execvp(parsed[0], parsed);
		printf("execvp failed\n");
	}else{	// parent
		wait(NULL);		// waiting for child to terminate 
		return;
	}
}


// Function where the multiple system commands are executed
void executeMultiCommands(int nc, int ns, char** commands, char* symbols){
	int i;
	char **cmdArgs;
	pid_t pid;
	
	for (i=0;i<nc;i++){
		
		cmdArgs = (char**)malloc(MAXCHAR * sizeof(char*));
		parseSpace(commands[i], cmdArgs);
	
		pid = fork();
		
		if (pid < 0){
			printf("Error! Failed to fork child :(\n");
			return;
		}else if (pid == 0){	// child
			//printf("Child [%d]\n", i);
			execvp(cmdArgs[0], cmdArgs);
			printf("execvp [%d] failed\n", i);
			exit(errno);			
		}else{	// parent
			int status;
			wait(&status);		// waiting for child to terminate 
			if(WIFEXITED(status)){	// returns a nonzero value if the child process terminated normally with exit
				// if WEXITSTATUS(status) = 0, the execvp was successful
         		//printf("child exited with = %d\n",WEXITSTATUS(status));
         		if (WEXITSTATUS(status) != 0 && symbols[i] == '&'){
         			printf("Symbol '&' prevents the execution of %s\n", commands[i+1]);
         			i++;
         		}
         	}	
			free(cmdArgs);	// free memoery to avoid segmentation faults
		}
	}
	return;
}


// Function for finding pipe 
int parsePipe(char* input, char** pipeArgs){ 
	int i; 
	for (i = 0; i < 2; i++) { 
		pipeArgs[i] = strsep(&input, "|");
		if (pipeArgs[i] == NULL) 
			break; 
	} 

	if (pipeArgs[1] == NULL) 
		return 0;	// returns 0 if no pipe is found. 
	else { 
		return 1;	// returns 1 if pipe is found
	} 
}


// Function where the piped system commands are executed 
void executePipeCommands(char** parsePipeRead, char** parsePipeWrite){ 
	// 0 is read end, 1 is write end 
	int pd[2]; 
	pid_t p1, p2; 

	if (pipe(pd) < 0){ 
		printf("\nPipe could not be initialized\n"); 
		return; 
	}
	 
	p1 = fork(); 
	
	if (p1 < 0) { 
		printf("\nfork failed in child p1\n"); 
		return; 
	}

	if (p1 == 0) { 
		// Child 1 executing 
		// Child 1 writes at the write end 
		close(pd[0]);					// close read proccess
		dup2(pd[1], STDOUT_FILENO);		// Redirects to STDOUT the pipe's write end
		dup2(pd[1], STDERR_FILENO);		// Redirects to STDERR the pipe's write end
		close(pd[1]);					// close write proccess 

		if (execvp(parsePipeRead[0], parsePipeRead) < 0) { 
			printf("\nPipe proccess: Could not execute command 1\n"); 
			exit(0); 
		} 
	}else{ 
		// Parent executing 
		p2 = fork(); 

		if (p2 < 0) { 
			printf("\nfork failed in child p2\n"); 
			return; 
		} 

		// Child 2 executing
		// Child 2 reads at the read end 
		if (p2 == 0) { 
			close(pd[1]);					// close write proccess 
			dup2(pd[0], STDIN_FILENO);		// Redirects to STDIN the pipe's read end
			dup2(pd[0], STDERR_FILENO);		// Redirects to STDERR the pipe's read end
			close(pd[0]); 					// close read proccess
			
			if (execvp(parsePipeWrite[0], parsePipeWrite) < 0) { 
				printf("\npipe proccess: Could not execute command 2\n"); 
				exit(0); 
			} 
		}else{ 
			// parent executing, waiting for two children 
			wait(NULL); 
			wait(NULL); 
		} 
	} 
} 


// Function for finding redirection ">" 
int parseRedirection(char* input, char** redirArgs){ 
	int i; 
	for (i = 0; i < 2; i++) { 
		redirArgs[i] = strsep(&input, ">");
		if (redirArgs[i] == NULL) 
			break; 
	} 

	if (redirArgs[1] == NULL) 
		return 0;	// returns 0 if no redirection is found. 
	else { 
		return 1;	// returns 1 if redirection is found
	} 
}


// Redirect output using ">"
void executeRedirection(char** parseRedir, char** redirArgs){
	int out = open(redirArgs[1], O_RDWR|O_CREAT|O_APPEND, 0600);
	
    if (out == -1){
    	perror("opening file for redirection");
    	return;
    }

    int err = open("cerr.log", O_RDWR|O_CREAT|O_APPEND, 0600);
    if (err == -1){
    	perror("opening cerr.log");
    	return;
    }

    int save_out = dup(fileno(stdout));
    int save_err = dup(fileno(stderr));

    if (dup2(out, fileno(stdout)) == -1){
    	perror("cannot redirect stdout");
    	return;
    }
    	
    if (dup2(err, fileno(stderr)) == -1){
    	perror("cannot redirect stderr");
    	return;
	}

	pid_t pid = fork(); 

	if (pid < 0) { 
		printf("\nfork failed in child in redirection function\n"); 
		return; 
	} 

	// Child reads at the read end 
	if (pid == 0){	
		if (execvp(parseRedir[0], parseRedir) < 0) { 
			printf("\nredirection proccess: Could not execute command '>' \n"); 
			exit(-1); 
		} 
	}else{ 
		// parent executing, waiting for the child
		wait(NULL);  
	}     
    
    
	
    fflush(stdout);
    close(out);
    fflush(stderr);
    close(err);

    dup2(save_out, fileno(stdout));
    dup2(save_err, fileno(stderr));

    close(save_out);
    close(save_err);
    return;
}



















