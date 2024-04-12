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
 * Somehow... I don't know if this still is an issue, but I'm too lazy to check
 * it rn.
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

//num <=7
int *conv_to_bin(int num){
    int *binary_arr = (int *)malloc(3 * sizeof(int));
    for (int i = 0; i < 3; i++) {
        binary_arr[3-1-i] = (num >> i) & 1;
    }
    return binary_arr;
}


char *convert_permission_to_string(int perms){
    // Integer definitions for permission bits. Also normalized it to 1-2-4.
    int special =((perms & S_ISUID)+(perms & S_ISGID)+(perms & S_ISVTX))/512;
    int user    = (perms & S_IRWXU)/64;
    int group   = (perms & S_IRWXG)/8;
    int others  = perms & S_IRWXO;

    // These are integer arrays of length 3, the bit representations of
    // respective variables.
    // ex: 3 = 011 it means rw-
    // read = 1, write = 2, exec = 4
    // sticky = 1, gid = 2, uid = 4
    int *user_bin = conv_to_bin(user);
    int *group_bin = conv_to_bin(group);
    int *others_bin = conv_to_bin(others);
    int *special_bin = conv_to_bin(special);// [sticky, gid, uid]

    /* arr = [
    * usr_read,    usr_write,    usr_exec
    * grp_read,    grp_write,    grp_exec
    * others_read, others_write, others_exec
    * SUID,        SGID,         Sticky bit
    * ]*/
    // 1 filetype, 3 user, 3 group, 3 others = 10
    char *arr = malloc(11 * sizeof(char)); 
    if(!arr){
        fprintf(stderr, "Memory can not be allocated.");
        return NULL;
    }
    memset(arr, '-', 10); //last byte should be null byte (\0)
    arr[10] = '\0';
 
    // Filetype 
    if(S_ISREG(perms)) arr[0] = '.';
    if(S_ISDIR(perms)) arr[0] = 'd';
    if(S_ISCHR(perms)) arr[0] = 'c';
    if(S_ISBLK(perms)) arr[0] = 'b';
    if(S_ISFIFO(perms)) arr[0] = 'p';

    // Owner user permissions
    if(user_bin[0]) arr[1] = 'r';
    if(user_bin[1]) arr[2] = 'w';
    if(user_bin[2]) arr[3] = 'x';

    // Owner group permissions
    if(group_bin[0]) arr[4] = 'r';
    if(group_bin[1]) arr[5] = 'w';
    if(group_bin[2]) arr[6] = 'x';

    // Others' permissions
    if(others_bin[0]) arr[7] = 'r';
    if(others_bin[1]) arr[8] = 'w';
    if(others_bin[2]) arr[9] = 'x';

    // GID, UID, Sticky bits
    if(special_bin[2] && (arr[9] == 'x')) arr[9] = 't';
    else if (special_bin[2]) arr[9] = 'T';
    if(special_bin[1] && (arr[6] == 'x')) arr[6] = 's';
    else if (special_bin[1]) arr[6] = 'S';
    if(special_bin[0] && (arr[3] == 'x')) arr[3] = 's';
    else if (special_bin[0]) arr[3] = 'S';
    
    free(user_bin);
    free(group_bin);
    free(others_bin);
    free(special_bin);
    return arr;
}


typedef struct node{
    char val[256];
    struct node *next;
} node;


node *create(char *val){
    node *ret = (node *)malloc(sizeof(node));
    if (ret != NULL) {
        // strcpy(ret->val, val);
        snprintf(ret->val, sizeof(ret->val), "%s", val);
        ret->next = NULL;
    }else{
        fprintf(stderr, "Memory cannot be allocated.\n");
        return NULL;
    }
    return ret;
}

void append(node *head, node *newNode){
    node *curNode = head;
    while(curNode->next != NULL){
        curNode = curNode->next;
    }
    curNode->next = newNode;
}

void printlist(node *head){
    node *tmp = head;
    if (tmp->next == NULL){
        printf("WTF\t");
    }
    printf("%s", tmp->val);
    while(tmp->next != NULL){
        tmp = tmp->next;
        printf("%s   ", tmp->val);
    }
}

void freelist(node *head){
    node *cur = head;
    node *next;
    while(cur != NULL){
        next = cur->next;
        free(cur);
        cur = next;
    }
}

char *removeNewline(char* str) {
    int length = strlen(str);
    for (int i = 0; i < length; i++) {
        if (str[i] == '\n') {
            str[i] = '\0'; 
            return str;
        }
    }
    return str;
}


// ls -l contains the followings:
//  * Permissions
//  * User and Group info
//  * Size
//  * Last modify date
//  * Filename (This is mandatory so we can assume it was already present in
//  the output string)
node *construct_list(node *string, struct stat sb){
    // Initial definitions
    char *time_str; 
    // Spesific definitions
    mode_t perms = sb.st_mode; // unsigned int
    char *perms_str = convert_permission_to_string(perms);
    struct group *grp = getgrgid(sb.st_gid);
    struct passwd *usr = getpwuid(sb.st_uid);
    char *grp_name = malloc(50);//= grp->gr_name;
    snprintf(grp_name, strlen(grp->gr_name)+10, "%-10s", grp->gr_name);
    char *usr_name = malloc(50);//= usr->pw_name;
    snprintf(usr_name, strlen(usr->pw_name)+10, "%-10s", usr->pw_name);
    off_t filesize = sb.st_size; // long
    struct timespec mtime = sb.st_mtim;
    time_t modif_time = mtime.tv_sec; // Seconds, long
    time_str = removeNewline(ctime(&modif_time));

    //Convert non-string ones to string
    char *size_str = convert_to_filesize(filesize);
    
    // Create the nodes
    node *perm_node = create(perms_str);
    node *usr_node = create(usr_name);
    node *grp_node = create(grp_name);
    node *size_node = create(size_str);
    node *time_node = create(time_str);

    append(string, perm_node);
    append(string, usr_node);
    append(string, grp_node);
    append(string, size_node);
    append(string, time_node);

    //Free mallocs 
    free(grp_name);
    free(usr_name);
    free(perms_str);
    free(size_str);
    return string;
}

node *add_inode(node *string, struct stat sb){
    ino_t inode = sb.st_ino;
    char *inode_str = malloc(12); // needs 8 but i choose 12
    snprintf(inode_str, 10, "%lu", inode);
    node *inode_node = create(inode_str);
    append(string, inode_node);
    // memcpy(string, inode_str, 10);
    free(inode_str);
    return string;
}


int main(int argc, char *argv[])
{
    // list, size, all, reverse, inode, time, Recursive
    int *opts = options(argc, argv);
    // int optind = opts[7];
    int retval = 0;
    // int offset = 0;
    for (int optind = opts[7]; optind < argc; optind++) {
        char *path = malloc(512);
        if(argv[optind] == NULL){
            getcwd(path, 512);
        }
        else strcpy(path, argv[optind]);
        add_slash(path);
        char *filepath = malloc(512);
        int path_length = strlen(path);
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
        printf("\n%s:\n", path);
        while(contents){
            node *printed_str = create("");
            filename = contents->d_name;
            memcpy(filepath+path_length, filename, strlen(filename)+1);
            s = stat(filepath, &sb);
            if(opts[4]){
                printed_str = add_inode(printed_str, sb);
            }
            if(opts[0]){
                printed_str = construct_list(printed_str, sb);
            }
            if(opts[2]){
                printlist(printed_str);
                printf("%s\n", filename);
            }
            else{
                if(filename[0] != '.'){
                    printlist(printed_str);
                    printf("%s\n", filename);
                }
            }
            contents = readdir(dirp);
            freelist(printed_str);
        }

        close_dir:
        closedir(dirp);
        

        //frees
        free(path);
        free(filepath);
        (void) s;
    }
    free(opts);
    return retval;
}
