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

enum events {
    PROC_CREAT,
    PROC_EXIT,
    SIGNAL_RECV,
    SIGNAL_SENT,
    FILE_MODF
};

struct eventsInfo {
    enum events event;
    unsigned instant;
    int pid;
    //info of the events
    char ** arg;
    int NumArgs;
    int exitStatus;
    int signal;
    char *fileChanged;
    mode_t oldPerm;
    mode_t newPerm;
};

int processRegister(struct eventsInfo eevee, int fileID);

int toOctalMode(mode_t oldMask, char mode[], mode_t * mask);

int ViewDirectoryRecursive(char s[], char newMode[], int Octal, int option);

int xmod(int argc, char* argv[], int fileID);
