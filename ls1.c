#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "dirent.h"
#include "stdlib.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "pwd.h"
#include "grp.h"
#include "time.h"

/*! TODO:
 * when one of the user or group name is too long, output is fucked. Align it.
 * Somehow...
 *
 * Implement recursive list 
 *
 * Implement giving more path like ls path1, path2, path3... 
 *
 */

int *options(int argc, char *argv[]){
  int *bool_arr = malloc(8 * sizeof(int));
  memset(bool_arr, 0, 8*sizeof(int));
  int c;
  while((c = getopt(argc,argv, "lsaritR")) != -1){
    switch (c) {
      case 'l':
        bool_arr[0] = 1;
        break;
      case 's':
        bool_arr[1] = 1;
        break;
      case 'a':
        bool_arr[2] = 1;
        break;
      case 'r':
        bool_arr[3] = 1;
        break;
      case 'i':
        bool_arr[4] = 1;
        break;
      case 't':
        bool_arr[5] = 1;
        break;
      case 'R':
        bool_arr[6] = 1;
        break;
    }
  }
  // last element is the first non-option argument. 
  bool_arr[7] = optind;
  return bool_arr;
}

char *convert_to_filesize(off_t size){
  char *result = (char *) malloc(sizeof(char) * 20);
  long kb = 1024;
  long mb = 1024 * 1024;
  long gb = 1024 * 1024 * 1024;
  float trunc_size = 0.0;
  char *str = malloc(4);
  memset(str, '\0', 4);
  if (size > gb) {
    trunc_size = (float) size / gb;
    memcpy(str, "Gb", 3);
  }
  else if (size > mb) {
    trunc_size = (float) size / mb;
    memcpy(str, "Mb", 3);
  }
  else if (size > kb) {
    trunc_size = (float) size / kb;
    memcpy(str, "Kb", 3);
  }
  else {
    trunc_size = size;
    memcpy(str, "B", 2);
  }
  // if(trunc_size - (int)trunc_size == 0){
  //   sprintf(result, "%-5d %-2s", (int)trunc_size, str);
  // }
  // else
    sprintf(result, "%-5.1f %-2s", trunc_size, str);
  free(str);
  return result;
}

// Needs strlen+1 size for filepath. 
// If we need to add something, we need more space ofc...
void add_slash(char *filepath){
  size_t size = strlen(filepath);
  if (filepath[size-1] == '/') return;
  filepath[size] = '/';
  filepath[size+1] = '\0';
}

// ls -l contains the followings:
//  * Permissions
//  * User and Group info
//  * Size
//  * Last modify date
//  * Filename (This was mandatory so we can assume it was already present in
//  the output string)
//
// How to print lots of different datatypes...
// Maybe convert all to char * but I guess there is a better way of doing it.
int construct_list(char *string, struct stat sb){
  int offst = 0;
  // Initial definitions
  char *perms_str = malloc(25); // I guess it has 21 bits but I am not sure
  int size_perms;

  int size_size;
  char *time_str; // Actually 24 was enough but just to be sure...
  int size_time;
  // Spesific definitions
  mode_t perms = sb.st_mode; // unsigned int
  struct group *grp = getgrgid(sb.st_gid);
  struct passwd *usr = getpwuid(sb.st_uid);
  char *grp_name = grp->gr_name;
  int size_grp = strlen(grp_name);
  char *usr_name = usr->pw_name;
  int size_usr = strlen(usr_name);
  off_t filesize = sb.st_size; // long
  struct timespec mtime = sb.st_mtim;
  time_t modif_time = mtime.tv_sec; // Seconds, long
  time_str = ctime(&modif_time);

  //Convert non-string ones to string
  snprintf(perms_str, 23, "%d ", perms);
  // snprintf(size_str, 13, "%lu ", filesize);
  char *size_str = convert_to_filesize(filesize);
  size_perms = strlen(perms_str)-1;
  size_size = strlen(size_str);
  size_time = strlen(time_str)-1;

  // Put variables into the memory
  memcpy(string+offst, perms_str, size_perms);
  offst += size_perms;
  memcpy(string+offst, "\t", 1);
  offst += 1;
  memcpy(string+offst, usr_name, size_usr);
  offst += size_usr;
  memcpy(string+offst, "\t", 1);
  offst += 1;
  memcpy(string+offst, grp_name, size_grp);
  offst += size_grp;
  memcpy(string+offst, "\t", 1);
  offst += 1;
  memcpy(string+offst, size_str, size_size);
  offst += size_size;
  memcpy(string+offst, "\t", 1);
  offst += 1;
  memcpy(string+offst, time_str, size_time);
  offst += size_time;
  //Free mallocs 
  free(perms_str);
  free(size_str);
  // free(time_str);
  return offst;
}

void add_inode(char *string, struct stat sb){
  ino_t inode = sb.st_ino;
  char *inode_str = malloc(12); // needs 8 but i choose 12
  snprintf(inode_str, 10, "%lu  ", inode);
  memcpy(string, inode_str, 10);
  free(inode_str);
}


int main(int argc, char *argv[])
{
  // list, size, all, reverse, inode, time, Recursive
  int *opts = options(argc, argv);
  int optind = opts[7];
  int retval = 0;
  int offset = 0;
  char *path = malloc(512);
  if(argv[optind] == NULL){
    getcwd(path, 512);
  }
  else strcpy(path, argv[optind]);
  add_slash(path);
  char *filepath = malloc(512);
  int path_length = strlen(path);
  char *printed_str = malloc(1024);
  memset(printed_str, '\0', 1024);
  memcpy(filepath, path, path_length);
  DIR *dirp = opendir(path);
  struct stat sb;
  int s = 0;
  // Check for any errors
  if(dirp == NULL){
    perror("opendir");
    retval = -1;
    goto close_dir;
  }
  char *filename;
  struct dirent *contents = readdir(dirp); // contains d_ino, d_name
  while(contents){
    offset = 0;
    filename = contents->d_name;
    memcpy(filepath+path_length, filename, strlen(filename)+1);
    s = stat(filepath, &sb);
    if(opts[4]){
      add_inode(printed_str+offset, sb);
      offset += 10;
    }
    if(opts[0]){
      offset += construct_list(printed_str+offset, sb);
    }
    if(opts[2]){
      printf("%s  %s\n", printed_str, filename);
    }
    else{
      if(filename[0] != '.')
        printf("%s  %s\n", printed_str, filename);
    }
    memset(printed_str, '\0', offset+100);
    contents = readdir(dirp);
  }

  close_dir:
  (void) s;
  closedir(dirp);
  
  free(printed_str);
  free(path);
  free(filepath);
  free(opts);
  return retval;
}
