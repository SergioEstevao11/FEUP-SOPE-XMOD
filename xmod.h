#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>


//s = directory; indent = 0
int ViewDirectoryRecursive(char s[], int indent);

int xmod(int argc, char* argv[], char* envp[]);

