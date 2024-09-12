#ifndef MYSHELL_H
#define MYSHELL_H
#define MAX_ARGS 64
#define MAX_ARG_LEN 32

void line_parser(char *line, char **split_line);
void tokenizer(char *line, char tokens[MAX_ARGS][MAX_ARG_LEN], int *lastStr);
int is_valid_command(char * command);
void execute_command(char ** command);
void execute_cd(char ** command);
#endif
