#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
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
  while (line[i] != '\0') {
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

void tokenizer_deadline(char *line, char tokens[MAX_ARGS][MAX_ARG_LEN], int* lastStr) {
  int i = 0, j = 0, k = 0;
  while (line[i] != '\0') {
    if (line[i] == '|') {
      j++;
      i++;
      k=0;
    } else {
      tokens[j][k] = line[i];
      k++;
      i++;
    }
  }
  *lastStr = j+1;
  tokens[j][k] = '\0';
}


void tokenizer_redirection(char *line, char split_line[MAX_ARGS][MAX_ARG_LEN], int *redirNo, int* numRedirCommands) {
  char delimiters[] = "<>";
  char* token;
  int i = 0, j=0, k=0, l=0;
  while (line[i] != '\0') {
    if (line[i] == '>' || line[i] == '<') {
      split_line[j][k] = '\0';
      redirNo[l++] = (line[i] == '>') ? 1 : 0;
      j++;
      i++;
      k=0;
      split_line[j][k] = line[i];
    } else {
      split_line[j][k] = line[i];
      i++;
      k++;
    }
    
    }
  *numRedirCommands = j+1;
    split_line[j][k] = '\0';
 }

void removeStartSpace(char *token) {
  int i = 0;
  int count = 0;
  while (token[i] == ' ') {
    i++;
  }
  while (token[i] != '\0') {
    token[count++] = token[i++];
  }
  token[count] = '\0';
}
char *removeLastSpace(char *str)
{
  char *end;
  while(isspace((unsigned char)*str)) str++;
  if(*str == 0) return str;
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  end[1] = '\0';
  return str;
}

int is_valid_command(char * command) {
  int i;
  for (i = 0; valid_commands[i] != NULL; i++) {
    if (strcmp(command, valid_commands[i]) == 0) {
        return 1;
      }
  }
  return 0;
}

void execute_command(char ** command, int i) {
  if (strcmp(command[0], "cd") == 0) {
    execute_cd(command);
  } else {
    pid_t pid = fork(); //a new child process, cloned from parent
    if (pid == 0) { //are we in child process?
      //printf("%d: ", i+1); //before any command print the number
      //fflush(stdout); //flush stdout before execvp replaces current process
      if (execvp(command[0], command) == -1) { //execute the command[0] command with the rest of the array as arguments
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

void execute_pipelines(char commands[][MAX_ARGS_LEN], int numCommands) {
  int pipefd[2 * (numCommands - 1)]; //4 commands->3 pipes, each pipe needs a pipefd[0] read and a pipefd[1] write
  //pipes for every command
  int i;
  for (i = 0; i < numCommands-1;i++) {
    if (pipe(pipefd + i * 2) == -1) {
      perror("pipe failed to create.");
      exit(EXIT_FAILURE);
    }
  }
  //for each pipe, the first command writes to stdin, the second pipe writes to stdout
  for (i = 0; i < numCommands; i++) {
    pid_t pid = fork(); //children for eac process
    if (pid == 0) { //check if its child
      //order is important, dup2() redirect must occur before execvp since execvp will cut the rest of the code
      if (i > 0) {  //redirect input from previous to stdin
	dup2(pipefd[(i-1)*2], STDIN_FILENO);
	//printf("\n");
      }
      if (i < numCommands - 1) { //if not last command, redirect output to stdout
	dup2(pipefd[i*2+1], STDOUT_FILENO); 
      }
      int j;
      for (j = 0; j < 2 * (numCommands - 1); j++) {
	close(pipefd[j]); //close pipes before execvp
      }
      //exec
      char *operations[MAX_ARGS] = {NULL};
      //printf("Splitting command: %s\n", commands[i][0]);
      line_parser(commands[i], operations);

      if (strcmp(operations[0], "grep") == 0) {
        // Shift existing arguments to make room for --line-buffered
        for (int k = MAX_ARGS - 1; k > 1; k--) {
          operations[k] = operations[k - 1];  // Shift arguments to the right
        }
        operations[1] = "--line-buffered"; // Insert --line-buffered after grep
	operations[2] = "--color=always";
      }
      
      if (execvp(operations[0], operations) == -1) {
	  perror("execvp failed.");
	  exit(EXIT_FAILURE);
	}
    }  else if (pid < 0) {
      perror("fork failed, at parent.");
      exit(EXIT_FAILURE);
    }
  }
  //close parents
  for (int i = 0; i < 2 * (numCommands-1); i++) {
    close(pipefd[i]);
  }
  for (int i = 0; i < numCommands; i++) {
    wait(NULL);
  }
}

void execute_redir_nopipes(char reDirOperations[][MAX_ARGS_LEN], int numRedirCommands, int* redirNo) {
  int fd; //file descriptor: redirection.
  pid_t pid = fork();
  int j;

  if (pid == 0) {
    for (int i = 1; i < numRedirCommands; i++) {
      removeStartSpace(reDirOperations[i]);
      char * fileToDirectInto = removeLastSpace(reDirOperations[i]);
      if (redirNo[i-1] == 0) {
        fd = open(fileToDirectInto, O_RDONLY);
        if (fd < 0) {
	  perror("Error opening the input file");
	  exit(EXIT_FAILURE);
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
	  perror("dup2 failed for stdin"); //redirect the stdin into the fd
	  exit(EXIT_FAILURE);
	}
        close(fd);
    } else if (redirNo[i - 1] == 1) {
        fd = open(fileToDirectInto, O_WRONLY | O_CREAT | O_TRUNC, 0644); //tags for stdout writing to a file
        if (fd < 0) {
          perror("Error opening the output file");
          exit(EXIT_FAILURE);
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
	  perror("dup2 failed for stdout");  // Redirect stdout to fd
	  exit(EXIT_FAILURE);
	}
        close(fd);
    }
  }
   char *operations[MAX_ARGS] = {NULL};
   line_parser(reDirOperations[0], operations);
   if (execvp(operations[0], operations) == -1) {
	  perror("execvp failed.");
	  exit(EXIT_FAILURE);
	  }
  } else if (pid < 0) { 
      perror("failed to fork");
      return;
  } else { 
      int status;
      waitpid(pid, &status, 0);
  }
}

int main(int argc, char *argv[]) {
  int n_prompt = 1; //default is to print the prompt
  if (argc > 1 && strcmp(argv[1],"-n")==0) n_prompt = 0;
  char line[MAX_LINE];

  while (1) {
    if (n_prompt) printf("my_shell$");
    if (fgets(line, MAX_LINE, stdin) == NULL) {
      printf("\nExiting shell. Thank you!!!!...\n");
      break;
    }
    line[strcspn(line, "\n")] = 0; //fgets reads a \n with enter. Remove it from the line.
    char *operations[MAX_ARGS] = {NULL};
    char tokens[MAX_ARGS][MAX_ARGS_LEN] = {{0}};
    int numCommands;
    tokenizer_deadline(line,tokens,&numCommands);
    int i = 0;
    char reDirOperations[MAX_ARGS][MAX_ARGS_LEN] = {{0}};
    //char test_str[50] = "hi>bye";
    int redirNo[50] = {-1};
    int numRedirCommands;
    tokenizer_redirection(line, reDirOperations, redirNo, &numRedirCommands);
    //printf("%s\n", shooooka[1]);
    //printf("%d\n",redirNo[0]);
    while (i < numCommands) {
      removeStartSpace(tokens[i]);
      i++;
    }
    if (numCommands == 1 && numRedirCommands == 1) {
      line_parser(tokens[0], operations);
      execute_command(operations, i);
    } else if (numCommands == 1 && numRedirCommands > 1) {
      execute_redir_nopipes(reDirOperations, numRedirCommands, redirNo);
    }
    else {
      execute_pipelines(tokens , numCommands);
    }
  }
  return 0;
}
