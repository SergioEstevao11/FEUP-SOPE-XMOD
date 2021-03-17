#ifndef SOPE_MP1_INCLUDE_XMOD_H_
#define SOPE_MP1_INCLUDE_XMOD_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <ctype.h>
#include <string.h>

#include <sys/wait.h>
#include <signal.h>

#include <dirent.h>
#include <sys/stat.h>

#include <errno.h>

#include "../include/xmod_aux.h"

const char *signame[] = {"INVALID", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1", "SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM", "SIGSTKFLT", "SIGCHLD", "SIGCONT", "SIGSTOP", "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGPOLL", "SIGPWR", "SIGSYS", NULL};

enum events {
    PROC_CREAT,
    PROC_EXIT,
    SIGNAL_RECV,
    SIGNAL_SENT,
    FILE_MODF
};

struct eventsInfo {
    FILE* file; 
    int hasFile;
    double instant; // Time Elapsed
    pid_t pidTarget; 

    //info of the events
    //PRO_CREATE
    char** arg;
    int numArgs;
	int nftot; // Total files checked 
	int nfmod; // Files modified
	int exitStatus;
	int signal;
    char *fileChanged;
    mode_t oldPerm; 
    mode_t newPerm;
};

struct eventsInfo eevee;

void sig_handler(int signal);

int signalSetup(void);

int chmod_handler(char *file, mode_t newperm, mode_t oldperm);

int processRegister(pid_t pid, enum events event);

int viewDirectoryRecursive(char s[], char newMode[], int Octal, int option);

int xmod(int argc, char* argv[]);

#endif  // SOPE_MP1_INCLUDE_XMOD_H_