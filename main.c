/*
 * @file         main.c
 * @description  source code of shell
 * @course       Operating Systems
 * @date         5/12/2021
 * @authors
 *  - Deniz Berfin Taştan
 *  - Eren Ugurlu
 *  - Furkan Cebar
 *  - Elif Saadet Tokuoğlu
 *  - Melih Tüfekçioğlu
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <limits.h>

#define COMMAND_SIZE 80 + 1//for /0
#define PARAMETER_COUNT 10 + 1//for NULL

//Executes the given program
pid_t Execute(char* parameters[PARAMETER_COUNT]);

//gets current working directory
char* getPath();

//variables for keeping prompt informations
char filepath[PATH_MAX];
char computer[HOST_NAME_MAX];
char user[LOGIN_NAME_MAX];

int main() {
	//saves last 10 child proccess id for 'showpid' command
	pid_t childs[10];
	int nextChild = 0;

	//gets path userName and computerName
	getlogin_r(user,LOGIN_NAME_MAX);
	gethostname(computer,HOST_NAME_MAX);
	getPath();

	while (1) {
		//prints colorful prompt
		printf("%s%s@%s%s:%s%s%s> ",
		       "\e[1;3;32m",user,computer,"\e[0m",
		       "\e[1;3;34m",filepath,"\e[0m");

		//gets command form stdin(standart input/terminal)
		char command[COMMAND_SIZE];
		fgets(command,COMMAND_SIZE,stdin);
		command[strlen(command) - 1] = '\0';

		//splits command into parameters
		char* parameters[PARAMETER_COUNT];
		parameters[0] = strtok(command," ");
		for (int i = 1; i < PARAMETER_COUNT - 1; ++i) {
			parameters[i] = strtok(NULL," ");
		}

		//if command is exit program will stop
		if (strcmp(parameters[0],"exit") == 0) {
			printf("exit\n");
			return 0;
		}

		//if command is cd changes relative directory to parameter[1]
		//  and update filepath
		else if (strcmp(parameters[0],"cd") == 0) {
			int errNo = chdir(parameters[1]);
			if (errNo != 0) {
				fprintf(stderr,"An error occurred! ");
				fprintf(stderr,"(ErrorCode:%d) \n",errNo);
			}
			getPath();
			//PWD asked in project document(used unsetenv cause setenv will create clones of path)
			unsetenv("PWD");
			char pwd[PATH_MAX];
			getcwd(pwd,PATH_MAX);
			setenv("PWD",pwd,0);
			printf("%s", getenv("PWD"));
		}

		//if command is showpid shows last 10 childs proccess id
		else if (strcmp(parameters[0],"showpid") == 0) {
			for (int i = nextChild - 1;; --i) {
				if (i ==-1) i =9;
				if (childs[i] != 0)
					printf("  %d\n", childs[i]);
				if (i == nextChild)
					break;
			}
		}

		//if command is not built-in Execute the program
		else {
			//adds id of the process to list
			childs[nextChild] = Execute(parameters);
			nextChild++;
			if (nextChild == 10)
				nextChild = 0;
		}
	}
}

//Executes the given program
pid_t Execute(char* parameters[PARAMETER_COUNT]) {
	//creates child process
	pid_t child_pid = fork();
	if (child_pid == 0) { // child process
		//execute the program and if program works properly it won't return
		execvp(parameters[0],parameters);
		//if program doesn't work sends a error
		fprintf(stderr,"%s: command not found\n",parameters[0]);
		abort();
	} else { //parent process
		//wait for child process to end
		waitpid(child_pid,NULL,0);
	}
	return child_pid;
}

//gets current working directory
char* getPath() {
	//gets current working directory with getcwd
	getcwd(filepath,PATH_MAX);
	//if filepath include home folder of user change it with '~'
	if (strncmp(filepath,"/home/",6) == 0) {
		char temp[PATH_MAX];
		strcpy(temp,filepath + 6);
		if (strncmp(temp, user, strlen(user)) == 0) {
			strcpy(filepath,temp + strlen(user) - 1);
			filepath[0] = '~';
		}
	}

}
