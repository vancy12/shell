#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_INPUT 1024
#define MAX_ARGS 100

char prompt[MAX_INPUT] = "\\w$ ";
char cwd[1024];
char path[MAX_INPUT] = "/usr/bin:/bin:/sbin";

void handle_error(int error_code, const char *context){

	switch(error_code){
        case EACCES:
            fprintf(stderr, "%s: Permission denied\n", context);
            break;
        case ENOMEM:
            fprintf(stderr, "%s: Out of memory\n", context);
            break;
        case ENOEXEC:
            fprintf(stderr, "%s: Exec format error\n", context);
            break;
        case ENOENT:
            fprintf(stderr, "%s: Command not found\n", context);
            break;
        default:
            fprintf(stderr, "%s: Unknown error (code %d)\n", context, error_code);
    }
}

void display_prompt(){

    if(strcmp(prompt, "\\w$ ") == 0 || strcmp(prompt, "\\w$") == 0){
        if(getcwd(cwd, sizeof(cwd)) != NULL){
            printf("%s$ ", cwd);
        } 
        else
            perror("getcwd error");
    } 
    else
        printf("%s", prompt);
}

void change_directory(char *dir){

    if(dir == NULL){
    	// get home folder path
        char *home = getenv("HOME");
        
        // handle error if home variable not set
        if(home == NULL){
            fprintf(stderr, "cd: HOME environment variable not set\n");
            return;
        }
        if(chdir(home) != 0){
            perror("cd");
        }
    }
    else if(chdir(dir) != 0)
        perror("cd");
}

int execute_command(char* cmd, char* args[], char* input_file, char* output_file, int append){

    int pid = fork();
    if(pid < 0){
        perror("fork error");
        return -1;
    }
    
    //child process
    else if(pid == 0){
        // Input redirection
        if(input_file){
            int fd_in = open(input_file, O_RDONLY);
            if(fd_in < 0){
                perror("Input redirection error");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        // Output redirection
        if(output_file){
            int fd_out;
            if(append){
                fd_out = open(output_file, O_WRONLY | O_CREAT | O_APPEND);
            }
            else 
                fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC);
            
            if(fd_out < 0){
                perror("Output redirection error");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        // execvp logic
        char* path_copy = strdup(path);
        char* dir = strtok(path_copy, ":");
        
        while(dir != NULL){
            char full_path[MAX_INPUT];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);
            execv(full_path, args);
            dir = strtok(NULL, ":");
        }
        
        free(path_copy);
        handle_error(errno, cmd);
        exit(EXIT_FAILURE);
    }
    
    // parent process
    else{
        int status;
        
        //waits for child process to complete execution
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

int parse_and_execute(char *input){

    char* args[MAX_ARGS];
    char* input_file = NULL;
    char* output_file = NULL;
    int append = 0;

    // Tokenize input for redirection symbols
    // find first occurence of > and < in string
    char* input_redirect = strchr(input, '<');
    char* output_redirect = strchr(input, '>');

    if(input_redirect){
    	// null terminate the first part/command
        *input_redirect = '\0';
        
        // get filename
        input_file = strtok(input_redirect + 1, " ");
    }

    if(output_redirect){
    	// if >> found then append
        if(*(output_redirect + 1) == '>'){
            append = 1;
            *output_redirect = '\0';
            output_file = strtok(output_redirect + 2, " ");
        }
        
        // if > found then truncate
        else{
            *output_redirect = '\0';
            output_file = strtok(output_redirect + 1, " ");
        }
    }

    // Tokenize command and arguments
    int arg_count = 0;
    char* token = strtok(input, " ");
    
    // MAX_ARGS - 1 is set as limit since if we have 99 arguments
    // the last should be a NULL pointer. So to leave space for it 
    // our terminating condition is MAX_ARGS -1
    
    while(token != NULL && arg_count < MAX_ARGS - 1){
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;

    if(arg_count == 0){
        return 0; 
    }

    // Handle some special commands
    if(strcmp(args[0], "exit") == 0){
    	printf("Exiting shell. Goodbye!!\n");
        exit(0);
    }
    else if(strcmp(args[0], "cd") == 0){
        change_directory(args[1]);
        return 0;
    }
    else if(strncmp(args[0], "PS1=", 4) == 0){
        strcpy(prompt, args[0] + 4);
        //printf("prompt is:%s \n", prompt);
        return 0;
    }
    else if(strncmp(args[0], "PATH=", 5) == 0){
        strcpy(path, args[0] + 5);
        return 0;
    }

    // Execute normal commands
    return execute_command(args[0], args, input_file, output_file, append);
}

int main(){
    char input[MAX_INPUT];

    while(1){
        display_prompt();
        if(fgets(input, MAX_INPUT, stdin) == NULL){
            printf("\nExiting shell. Goodbye!\n");
            break;
        }

        // Replace newline character at the end
        // with null terminator
        input[strcspn(input, "\n")] = 0;

        // Parse and execute the command
        parse_and_execute(input);
    }
    return 0;
}

