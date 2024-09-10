#ifndef MYSHELL_H
#define MYSHELL_H

void line_parser(char *line, char **split_line);
int is_valid_command(char * command);
void execute_command(char ** command);
void execute_cd(char ** command);
void execute_commandd(char ** command);
void execute_ls(char ** command);
#endif
