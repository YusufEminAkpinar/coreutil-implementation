#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "limits.h"


void read_file_content(const char *path){
  FILE *file = fopen(path, "r");
	char *line = malloc(sizeof(char) * 5000);
	if (file != NULL) {
		while (fgets(line, INT_MAX, file) != NULL) {
			printf("%s", line);
		}
	}
  free(line);
}


int main(int argc, char *argv[])
{
  const char *path = "/home/hardal/test/file.js";
  // FILE *file = fopen(path, "r");
  read_file_content(path);
  return 0;
}
