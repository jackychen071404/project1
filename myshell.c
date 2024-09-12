#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include "myshell.h"
#define MAX_LINE 512
#define MAX_ARGS 64
#define MAX_ARGS_LEN 32

const char *valid_commands[] = {
  "ls","cd", "pwd", "mkdir", "rmdir", "rm", "cp", "mv", "echo", "cat", "grep", "man", "wc", "touch", NULL
};

const char *valid_operators[] = {
  "<", ">", "|", "&"
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

void tokenizer(char *line, char tokens[MAX_ARGS][MAX_ARG_LEN], int* lastStr) {
  int i = 0, j = 0, k = 0;
  while (line[i] != NULL) {
    if (line[i] == '>' || line[i] == '<' || line[i] == '|') {
      if (k > 0) {
	tokens[j][k] = '\0'; //end the character of the last arg, before start new
	j++;
	k = 0;
      }
      tokens[j][0] = line[i];
      tokens[j][1] = '\0';
      j++;
      i++;
    } else {
      tokens[j][k] = line[i];
      k++;
      i++;
    }
  }
  *lastStr = j+1;
  tokens[j][k] = '\0';
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
    char tokens[MAX_ARGS][MAX_ARGS_LEN];
    int lastStr;
    tokenizer(line,tokens,&lastStr);
    int i = 0;
    while (i < lastStr) {
      printf("%s\n", tokens[i]);
      i++;
    }
    line_parser(tokens[0], operations);
    /*if (operations[0] == NULL) {
      continue; //do nothing if blank
      }*/
    //if (is_valid_command(operations[0])) {
    execute_command(operations);
      //printf("\nAwesome\n");
      //} else {
      //printf("ERROR!: '%s' is not a valid command\n", operations[0]);
      //}
  }
  return 0;
}
