#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "limits.h"
#include "sys/stat.h"

void read_file_content(const char *path);
// void read_from_stdin();


int main(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++) {
    read_file_content(argv[i]);
  }
  return 0;
}

// void read_from_stdin(){
//   // char *buf = malloc(sizeof(char) * )
//   // fgets
// }

void read_file_content(const char *path){
  struct stat sb;
  stat(path, &sb);
  FILE *file = fopen(path, "r");
  if (S_ISDIR(sb.st_mode)){
    printf("%s: Is a directory.\n", path);
    return;
  }
	if (file != NULL) {
	  char *line = malloc(sizeof(char) * 5000);
		while (fgets(line, INT_MAX, file) != NULL)
			printf("%s", line);
    free(line);
	} else printf("%s: No such file.\n", path);
}


