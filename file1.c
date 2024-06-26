
// /usr/share/mime/magic file has all the info you need to know. basically it is
// the database for file-header correspondance

/* How to read the magic file?
 * I guess it organized something like this:
 * Lines that start with [** is obviously the filetypes.
 * And the lines below them till the other [** is headers.
 * For example:
   ** [60:application/msword]
   ** >0= 1¾  
   ** >0= PO^Q`
   ** >0= þ7 #
   ** >0= Û¥-   
   ** >2112= 	MSWordDoc
   ** >2108= 	MSWordDoc
   ** >2112= Microsoft Word document data
   ** >546= bjbj
   ** >546= jbjb
 * all of these lines start with > indicates different headers,
 * if first n bytes of a file has one of those, then it means this is a msword
 * file.
 */

/* So...
 * Maybe just create a hashmap that match the [** lines with > lines.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define header_size 50
#define arr_size(arr) (sizeof(arr) / sizeof(arr[0]))

void read_section(FILE *file);


int main(int argc, char *argv[])
{
  (void) argc;
  // Open the magic file
  // FILE *magic = fopen("/data/data/com.termux/files/usr/share/mime/magic", "r");
  FILE *magic = fopen("/usr/share/mime/magic", "r");
  if (magic == NULL) return 1;
  char *line = malloc(header_size);

  for (int i = 1; i < argc; i++) {
    FILE *inputfile = fopen(argv[i], "rb");
    if (inputfile == NULL) return 1;
    printf("For the file %s:\t", argv[i]);
    read_section(inputfile);
    fclose(inputfile);
  }
  
  free(line);
  fclose(magic);
  return 0;
}


void read_section(FILE *file){
  size_t ret;
  unsigned char buf[4];
  ret = fread(buf, sizeof(*buf), arr_size(buf), file);
  if (ret != arr_size(buf)){
    fprintf(stderr, "fread() failed %zu\n", ret);
    exit(EXIT_FAILURE);
  }
  printf("ELF magic is: %#04x%02x%02x%02x\n", buf[0], buf[1], buf[2], buf[3]);
}
