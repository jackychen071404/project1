#ifndef MYSHELL_H
#define MYSHELL_H

void line_parser(char *line, char **split_line, char **pipeline);
int is_valid_command(char * command);
void execute_command(char ** command);
void execute_cd(char ** command);
#endif
