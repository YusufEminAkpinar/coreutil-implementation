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

/* New idea:
 * Just construct it with linked list.
*/

typedef struct node{
    char *val;
    struct node *next;
} node;

node create(char *val){
    node ret;
    // strcpy(ret.val, val);
    ret.val = strdup(val);
    ret.next = NULL;
    return ret;
}

void append(node *head, node *newNode){
    node *curNode = head;
    while(curNode->next != NULL){
        curNode = curNode->next;
    }
    curNode->next = newNode;
}

/*
  * Memory layout of a node struct:
  * |--string(val)--|--next(node *)--|
  * that string takes strlen(val) bytes,
  * and 
  *
*/


// that printing ...->val fucks it up somehow...
void printlist(node *head){
    node *tmp = head;
    int i = 0;
    // printf("\nInside PRINT function:\n");
    // printf("Our head node is: %s\n", head->val);
    if (tmp->next == NULL){
        printf("WTF\t");
    }
    while(tmp->next != NULL){
        i++;
        printf("%d) %s\t",i, tmp->val);
        tmp = tmp->next;
    }
    printf("%s\t", tmp->val);
    // printf("\nOutside PRINT function:\n");
}

void fill(node head){
    node a = create("First fill");
    node b = create("Second fill");
    node c = create("Third fill");
    
    append(&head, &a);
    append(&head, &b);
    append(&head, &c);
}


void freelist(node *head){
    node *tmp = head;
    printf("tmp is: %s\n", tmp->val);
    while(tmp->next != NULL){
        printf("node with value %s is freed.\n", tmp->val);
        free(tmp->val);
        tmp = tmp->next;
    }
    free(tmp->val);
}

void resetlist(node *head){
    node *tmp = head;
    while(tmp->next != NULL){
        memset(tmp->val, '\0', strlen(tmp->val));
        tmp = tmp->next;
    }
}

// ls -l contains the followings:
//  * Permissions
//  * User and Group info
//  * Size
//  * Last modify date
//  * Filename (This is mandatory so we can assume it was already present in
//  the output string)
//
// How to print lots of different datatypes...
// Maybe convert all to char * but I guess there is a better way of doing it.
node construct_list(node string, struct stat sb){
    // int offst = 0;
    // Initial definitions
    char *perms_str = malloc(25); // I guess it has 21 bits but I am not sure
    char *time_str; 
    // Spesific definitions
    mode_t perms = sb.st_mode; // unsigned int
    struct group *grp = getgrgid(sb.st_gid);
    struct passwd *usr = getpwuid(sb.st_uid);
    char *grp_name = grp->gr_name;
    char *usr_name = usr->pw_name;
    off_t filesize = sb.st_size; // long
    struct timespec mtime = sb.st_mtim;
    time_t modif_time = mtime.tv_sec; // Seconds, long
    time_str = ctime(&modif_time);

    //Convert non-string ones to string
    snprintf(perms_str, 23, "%d ", perms);
    // snprintf(size_str, 13, "%lu ", filesize);
    char *size_str = convert_to_filesize(filesize);
    
    // Create the nodes
    node head = string;
    node perm_node = create(perms_str);
    node usr_node = create(usr_name);
    node grp_node = create(grp_name);
    node size_node = create(size_str);
    node time_node = create(time_str);

    append(&head, &perm_node);
    append(&head, &usr_node);
    append(&head, &grp_node);
    append(&head, &size_node);
    append(&head, &time_node);
    // string.next = &perm_node;
    // perm_node.next = &usr_node;
    // usr_node.next = &grp_node;
    // grp_node.next = &size_node;
    // size_node.next = &time_node;

    //Free mallocs 
    free(perms_str);
    free(size_str);
    return head;
}

void add_inode(node *string, struct stat sb){
  ino_t inode = sb.st_ino;
  char *inode_str = malloc(12); // needs 8 but i choose 12
  snprintf(inode_str, 10, "%lu  ", inode);
  node inode_node = create(inode_str);
  append(string, &inode_node);
  // memcpy(string, inode_str, 10);
  free(inode_str);
}


int main(int argc, char *argv[])
{
    // list, size, all, reverse, inode, time, Recursive
    int *opts = options(argc, argv);
    int optind = opts[7];
    int retval = 0;
    // int offset = 0;
    char *path = malloc(512);
    if(argv[optind] == NULL){
        getcwd(path, 512);
    }
    else strcpy(path, argv[optind]);
    add_slash(path);
    char *filepath = malloc(512);
    int path_length = strlen(path);
    node printed_str = create("Testing2");
    // printed_str.val = malloc(1024);
    // memset(printed_str.val, '\0', 1024);
    // strcpy(printed_str.val, "Testing");
    // memset(printed_str, '\0', 1024);
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
    // node filenode;
    while(contents){
        // offset = 0;
        filename = contents->d_name;
        // memset(&filenode, '\0', strlen(filename));
        // filenode = create(filename, NULL);
        memcpy(filepath+path_length, filename, strlen(filename)+1);
        s = stat(filepath, &sb);
        if(opts[4]){
            add_inode(&printed_str, sb);
            // offset += 10;
        }
        if(opts[0]){
            // printf("Construction of list has been started.\n");
            // printed_str.next = NULL;
            printed_str = construct_list(create(""), sb);
            // printf("\n LIST HAS BEEN CONSTRUCTED \n");
        }
        if(opts[2]){
          // printf("%s  %s\n", printed_str.val, filename);
          // append(&printed_str, &filenode);
            printf("%s --> ", filename);
            // printlist(&printed_str);
            printf("\n");
        }
        else{
            if(filename[0] != '.'){
                printf("%s --> ", filename);
                printlist(&printed_str);
                printf("\n");
            }
        }
        // memset(&printed_str, '\0', offset+100);
        contents = readdir(dirp);
    }

    close_dir:
    (void) s;
    closedir(dirp);

    // free(&printed_str);
    // freelist(&printed_str);
    free(path);
    free(filepath);
    free(opts);
    return retval;
}
