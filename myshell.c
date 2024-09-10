#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#define MAX_LINE 512

int main() {
  char line[MAX_LINE];
  while (1) {
    printf("my_shell$");
    fgets(line, MAX_LINE, stdin);
    printf("\n%s",line);
  }
  return 0;
}
