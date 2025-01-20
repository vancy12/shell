#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_LENGTH 1024

char prompt[MAX_LENGTH] = "\\w$";
char cwd[MAX_LENGTH];

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
		
		printf("%s\n", input);
	
	}
	return 0;
}





