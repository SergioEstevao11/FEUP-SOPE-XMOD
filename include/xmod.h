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

// Holds most signals names in the index corresponding to their signal number
const char *signame[] = {"INVALID", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1", "SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM", "SIGSTKFLT", "SIGCHLD", "SIGCONT", "SIGSTOP", "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGPOLL", "SIGPWR", "SIGSYS", NULL};

// Enum that stores the events that may occur throughout the execution
enum events {

    PROC_CREAT, // Process created
    PROC_EXIT,  // Process finished
    SIGNAL_RECV, // Signal received
    SIGNAL_SENT, // Signal sent
    FILE_MODF // File modified

};

// Struct that stores all the relevant info about different events
struct eventsInfo {

    FILE* file; // File to write the event info
    int hasFile; // Signals if the environment variable LOG_FILENAME is defined: 1 if it is, 0 otherwise
    char * invokedFile; // Name of the file/directory that originated the process

    double instant; // Time elapsed since the beggining of the execution

    // PROC_CREATE
    char** arg; // Stores all the arguments that create the process
    int numArgs; // Number of arguments that create the process

    // PROC_EXIT
	int exitStatus; // Exit status of the process

    //SIGINT
	int nftot; // Total number of files found until register
	int nfmod; // Number of files modified

    //SIGNAL_RECEIVED and SIGNAL_SENT
	int signal; // Signal received or sent by the process
    pid_t pidTarget; // Process Identifier of the target process that the signal is sent to 

    //FILE_MODF
    char *fileChanged; // Name of the file that was modified
    mode_t oldPerm; // File's old permissions
    mode_t newPerm; // File's new permissions

};

//Global variable that holds the struct eventsInfo
struct eventsInfo infoReg;

// ---------------------------
//
// xmod.c functions
//
// ---------------------------

/** 
 * @brief Redirects most signals to sig_handler using sigaction
 * 
 * @return Returns 0 if no errors occurred, 1 otherwise
 */
int signalSetup(void);

/**
 * @brief Handles a signal received and sends other signals accordingly
 * 
 * @param signal Signal received
*/
void sigHandler(int signal);

/**
 * @brief Writes to log file an event from a specific process
 * 
 * @param pid Proccess Identifier
 * @param event Type of event to register
 * 
 * @return Returns 0 if the event is written to the file succesfully, 1 if the environment variable LOG_FILENAME is not defined (doesn't write in file)
 */
int processRegister(pid_t pid, enum events event);

/**
 * @brief Function that calls chmod and interprets its result
 * 
 * @param file Target file to change permissions
 * @param newperm File's new permissions to be applied
 * @param oldperm File's previous permissions
 * 
 * @return Returns 0 if no errors occurred, 1 otherwise
 */
int invokeChmod(char *file, mode_t newperm, mode_t oldperm);

/**
 * @brief Function that handles the recursive option "-R" where a directory tree is processed
 *
 * @param s Name of the path of the file passed as argument
 * @param newMode Mode which should be applied recursively to the files
 * @param isOctal 0 if it is in octal mode, 1 otherwise
 * @param option v for verbose option, c for changes option
 *
 * @return Returns 0 if no errors occurred, 1 otherwise
 */
int directoryRecursive(char s[], char newMode[], int isOctal, int option);

/**
 * @brief Main xmod function that parses the command line arguments and changes the permissions of the files, directories and symbolic links referenced.
 * 
 * @param argc Number of command line arguments. 
 * @param argv Array of strings with the command line arguments.
 *
 * @return Returns 0 if no errors occurred, 1 otherwise.
 */
int xmod(int argc, char* argv[]);

#endif  // SOPE_MP1_INCLUDE_XMOD_H_