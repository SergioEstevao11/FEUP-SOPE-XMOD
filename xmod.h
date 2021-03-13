#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>


#define MAX_ARGS 6
#define MIN_ARGS 3
#define USER_MASK 0x3F
#define GROUP_MASK 0x1C7
#define OTHERS_MASK 0x1F8
#define ALL_MASK 0x1FF
#define R_BIT 0x4
#define W_BIT 0x2
#define X_BIT 0x1

int ViewDirectoryRecursive(char s[], int indent);

int xmod(int argc, char* argv[], char* envp[]);

int toOctalMode(mode_t oldMask, char mode[], mode_t * mask);
