#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LENGTH 1024
#define MAX_ARGS 100

char prompt[MAX_LENGTH] = "\"\\w$\"";
char cwd[MAX_LENGTH];
char path[MAX_LENGTH] = "/usr/bin:/bin:/sbin";

void show_prompt(){
	// if it is default prompt show cwd
	if(strcmp(prompt, "\"\\w$\"") == 0){
		if(getcwd(cwd, MAX_LENGTH) != NULL){
			printf("%s$ ", cwd);
		}
	}
	else{
		printf("%s ", prompt);
	}
}

void change_directory(char* dir){
	// if only cd is given
	char* home = getenv("HOME");
	if(dir == NULL){
		if(!home){
			printf("HOME environment variable not set\n");
			return;
		}
		chdir(home);
	}
	// else if path given
	else{
		
		if(dir[0] == '~'){
			if(!home){
				printf("HOME environment variable not set\n");
				return;
			}
			char expanded_path[MAX_LENGTH];
			snprintf(expanded_path, sizeof(expanded_path), "%s/%s", home, dir + 1);
			dir = strdup(expanded_path);
		}
		chdir(dir);
	}
		
	
	return;
}

int execute(char* cmd, char* argv[], char* input_file, char* output_file, int append){

	int pid = fork();
	if(pid < 0){
		printf("Fork error!");
		exit(EXIT_FAILURE);
	}
	// child process
	else if(pid == 0){
		// if input redirection
		if(input_file){
			int fd_1 = open(input_file, O_RDONLY);
			if(fd_1 < 0){
				printf("input redirection error!\n");
				exit(EXIT_FAILURE);
			}
			// old file descriptor -> new file descriptor
			dup2(fd_1, STDIN_FILENO);
			close(fd_1);
		}
		
		if(output_file){
			int fd_2;
			
			if(append)
				fd_2 = open(output_file, O_WRONLY | O_CREAT | O_APPEND);
			else
				fd_2 = open(output_file, O_WRONLY | O_CREAT | O_TRUNC);
				
			if(fd_2 < 0){
				printf("output redirection error!\n");
				exit(EXIT_FAILURE);
			}
			// old file descriptor -> new file descriptor
			dup2(fd_2, STDOUT_FILENO);
			close(fd_2);
		}
		
		// execvp using execv
		char* path_copy = strdup(path);
		char* start = strtok(path_copy, ":");
		
		// handle ~ in arguments
		
		char* home = getenv("HOME");
				
		for(int i = 0; argv[i] != NULL; i++){
            if(argv[i][0] == '~'){
                char expanded_arg[MAX_LENGTH];
                snprintf(expanded_arg, sizeof(expanded_arg), "%s%s", home, argv[i] + 1);
                argv[i] = strdup(expanded_arg);
            }
        }
		
		while(start != NULL){
			char full_exec_path[MAX_LENGTH];
			// searching for executable file --> /usr/bin/ls
			// /usr/bin --> start and ls --> cmd
			snprintf(full_exec_path, sizeof(full_exec_path), "%s/%s", start, cmd);
			execv(full_exec_path, argv);
			start = strtok(NULL, ":");
		}
	}
	
	// parent process
	else{
		int wstatus;
		waitpid(pid, &wstatus, 0);
		return WEXITSTATUS(wstatus);
	}
	
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
    		append = 0;
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
   	/*int i =0;
   	while(argv[i] != NULL){
   		printf("%s ", argv[i]);
   		i++;
   	}
   	printf("\n");*/
   	
   	// special commands
   	if(strcmp(argv[0], "exit") == 0){
   		printf("Exiting...\n");
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
   		char* temp_prompt = argv[0] + 4;
   		
   		if(strcmp(temp_prompt,"\"\\w$\"") && temp_prompt[0] == '"' && temp_prompt[strlen(temp_prompt) - 1] == '"'){
   			temp_prompt[strlen(temp_prompt) - 1] = '\0';
   			temp_prompt++;
   		}
   		
   		strcpy(prompt, temp_prompt);
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
			printf("Exiting...\n");
			exit(0);
		}
		int index = strcspn(input, "\n");
		input[index] = '\0';
		
		parse_input(input);
		//printf("%s\n", input);
	
	}
	return 0;
}





