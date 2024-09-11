#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include "myshell.h"
#define MAX_LINE 512
#define MAX_ARGS 64

const char *valid_commands[] = {
  "ls","cd", "pwd", "mkdir", "rmdir", "rm", "cp", "mv", "echo", "cat", "grep", "man", "wc", "touch", NULL
};

void line_parser(char* line, char**split_line) {
  char *split = strtok(line, " ");
  int i = 0;
  while (split != NULL) {
    split_line[i++] = split;
    split = strtok(NULL , " "); //subsequent calls of strtok take the other fragments of space
  }
  split_line[i] = NULL; //null terminate end of char array
  //printf("%s", split);
}

int is_valid_command(char * command) {
  for (int i = 0; valid_commands[i] != NULL; i++) {
    if (strcmp(command, valid_commands[i]) == 0) {
	return 1;
      }
  }
  return 0;
}

void execute_command(char ** command) {
  if (strcmp(command[0], "cd") == 0) {
    execute_cd(command);
  } else {
    pid_t pid = fork(); //a new child process, cloned from parent
    if (pid == 0) { //are we in child process?
      if (execvp(command[0], command) == -1) {
	perror("execvp failed."); //let the child execute mkdir, in case it fails so parent shell wont be affected
      }
      exit(EXIT_FAILURE);
    } else if (pid > 0) { //are we in parent process?
      int status;
      waitpid(pid, &status, 0); //status holds the child process status, and 0 means to wait for this child to terminate
    } else {
      perror("fork failed.");
    }
  }
}

void execute_cd(char ** command) {
  if (command[1] == NULL) {
    perror("cd: missing argument");
  } else if (chdir(command[1]) != 0) { //change system directory
    perror("cd failed, directory not found");
  }
}
int main() {
  char line[MAX_LINE];
  
  while (1) {
    printf("my_shell$");
    if (fgets(line, MAX_LINE, stdin) == NULL) {
      printf("\nExiting shell. Thank you!!!!...\n");
      break;
    }
    line[strcspn(line, "\n")] = 0; //fgets reads a \n with enter. Remove it from the line.
    char *operations[MAX_ARGS];
    line_parser(line, operations);
    /*for (int i = 0; operations[i] != NULL; i++) {
            printf(" %s",operations[i]);
	    }*/
    if (operations[0] == NULL) {
      continue; //do nothing if blank
    }
    if (is_valid_command(operations[0])) {
      execute_command(operations);
      //printf("\nAwesome\n");
    } else {
      printf("ERROR!: '%s' is not a valid command\n", operations[0]);
    }
  }
  return 0;
}
