#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_LENGTH 1024
#define MAX_ARGS 100

char prompt[MAX_LENGTH] = "\\w$";
char cwd[MAX_LENGTH];
char path[MAX_LENGTH] = "/usr/bin:/bin:/sbin";

void show_prompt(){
	// if it is default prompt show cwd
	if(strcmp(prompt, "\\w$") == 0){
		if(getcwd(cwd, MAX_LENGTH) != NULL){
			printf("%s$ ", cwd);
		}
	}
	else{
		printf("%s$ ", prompt);
	}
}

void change_directory(char* dir){
	return;
}

int execute(char* cmd, char* argv[], char* input_file, char* output_file, int append){
	return 0;
}

int parse_input(char input[]){

	char* input_file = NULL;
	char* output_file = NULL;
	int append;
	
	// check for input and output redirection
    char* input_redirection	= strchr(input, '<');
    char* output_redirection = strchr(input, '>');
    
    if(input_redirection){
    	// command
    	*input_redirection = '\0'; 
    	input_file = strtok(input_redirection + 1, " ");
    }
    if(output_redirection){
    	if(*(output_redirection + 1) == '>'){
    		append = 1;
    		*output_redirection = '\0';
    		output_file = strtok(output_redirection + 2, " ");
    	}
    	else{
    		*output_redirection = '\0';
    		output_file = strtok(output_redirection + 1, " ");
    	}
    }
    
    //if(input_file != NULL)
    	//printf("%s", input_file);
    
    // tokeinzing the command given by blank spaces
    char* token = NULL;
    char* argv[MAX_ARGS];
    int argument_count = 0;
    
    token = strtok(input, " ");
    while(token != NULL && argument_count < MAX_ARGS){
    	argv[argument_count] = token;
    	argument_count++;
    	token = strtok(NULL, " ");
    }
    
    // null terminate argv
    argv[argument_count] = NULL;
    
    // debug
   	int i =0;
   	while(argv[i] != NULL){
   		printf("%s ", argv[i]);
   		i++;
   	}
   	printf("\n");
   	
   	// special commands
   	if(strcmp(argv[0], "exit") == 0){
   		printf("\nExiting...\n");
   		exit(0);
   	}
   	else if(strcmp(argv[0], "cd") == 0){
   		change_directory(argv[1]);
   		return 0;
   	}
   	else if(strncmp(argv[0], "PATH=", 5) == 0){
   		strcpy(path, argv[0]+5);
   		return 0;
   	}
   	else if(strncmp(argv[0], "PS1=", 4) == 0){
   		strcpy(prompt, argv[0] + 4);
   		return 0;
   	}
   	
    return execute(argv[0], argv, input_file, output_file, append);

}


int main(){

	char input[MAX_LENGTH];
	
	while(1){
		//printf("prompt>");
		show_prompt();
		if(fgets(input, MAX_LENGTH, stdin) == NULL){
			printf("\nExiting...\n");
			exit(0);
		}
		int index = strcspn(input, "\n");
		input[index] = '\0';
		
		parse_input(input);
		//printf("%s\n", input);
	
	}
	return 0;
}





