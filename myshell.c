#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "myshell.h"
#define MAX_LINE 512
#define MAX_ARGS 64

const char *valid_commands[] = {
  "ls","cd", "pwd", "mkdir", "rmdir", "rm", "cp", "mv", "echo", "cat", "grep", "man", NULL
};

int is_valid_command(char * command) {
  for (int i = 0; valid_commands[i] != NULL; i++) {
    if (strcmp(command, valid_commands[i]) == 0) {
	return 1;
      }
  }
  return 0;
}

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
    for (int i = 0; operations[i] != NULL; i++) {
            printf(" %s",operations[i]);
    }
    if (operations[0] == NULL) {
      continue; //do nothing if blank
    }
    if (is_valid_command(operations[0])) {
      printf("\nAwesome\n");
    } else {
      printf("\nERROR!: %s is not a valid command\n", operations[0]);
    }
  }
  return 0;
}
