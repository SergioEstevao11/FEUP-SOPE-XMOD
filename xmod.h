#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <ctype.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <dirent.h>
#include <sys/stat.h>

#include <errno.h>

#include "xmod_macros.h"

int toOctalMode(mode_t oldMask, char mode[], mode_t * mask);

int ViewDirectoryRecursive(char s[], char newMode[], int Octal, int option);

int xmod(int argc, char* argv[], char* envp[]);
