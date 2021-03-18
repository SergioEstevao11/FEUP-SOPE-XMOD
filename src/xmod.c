#include "../include/xmod.h"

int signalSetup(void){
    struct sigaction sig, old_action;
    sigset_t smask;  

    sigemptyset(&smask);

    sig.sa_mask = smask;  
    sig.sa_flags = SA_RESTART;
    sig.sa_handler = sig_handler;

    if (sigaction(SIGINT, &sig, &old_action) == -1){
        perror("Error in sigaction, SIGINT");
        return 1;
    }
    if (sigaction(SIGHUP, &sig, &old_action) == -1){
        perror("Error in sigaction, SIGHUP");
        return 1;
    }
    if (sigaction(SIGUSR1, &sig, &old_action) == -1){
        perror("Error in sigaction, SIGUSR1");
        return 1;
    }
    if (sigaction(SIGQUIT, &sig, &old_action) == -1){
        perror("Error in sigaction, SIGQUIT");
        return 1;
    }
    if (sigaction(SIGUSR2, &sig, &old_action) == -1){
        perror("Error in sigaction, SIGUSR2");
        return 1;
    }
    if (sigaction(SIGPIPE, &sig, &old_action) == -1){
        perror("Error in sigaction, SIGPIPE");
        return 1;
    }
    if (sigaction(SIGALRM, &sig, &old_action) == -1){
        perror("Error in sigaction, SIGALRM");
        return 1;
    }
    if (sigaction(SIGTERM, &sig, &old_action) == -1){
        perror("Error in sigaction, SIGTERM");
        return 1;
    }
    if (sigaction(SIGCHLD, &sig, &old_action) == -1){
        perror("Error in sigaction, SIGCHLD");
        return 1;
    }

    return 0;
}

void sig_handler(int signal) { 
    char input[MAX_BUF]; // Stores the user input

    infoReg.signal = signal; // Stores the signal in the struct
    processRegister(getpid(), SIGNAL_RECV); 
    
    if (signal == SIGINT) { // Only SIGINT is treated
        printf("\n%d ; %s ; %d ; %d\n", getpid(), infoReg.invokedFile, infoReg.nftot, infoReg.nfmod);

        if (getpgid(getpid()) == getpid()) { // Parent process is the only one who checks if the user wants to continue

            do {
                printf("Do you want to terminate the program? (y/n) ");
                scanf("%s", input);
            } while ( strcmp(input, "y") != 0 && strcmp(input, "n") != 0 );

            if (input[0] == 'y') {
                infoReg.signal = SIGKILL;
                infoReg.pidTarget = 0;
                processRegister(getpid(),SIGNAL_SENT);
                kill(0,SIGKILL); // Sends SIGKILL signal to every process
            } else {
                infoReg.signal = SIGCONT;
                infoReg.pidTarget = 0;
                processRegister(getpid(),SIGNAL_SENT);
                kill(0,SIGCONT); // Sends SIGCONT signal to every process
            }
        } else { // If process isn't parent process 
            infoReg.signal = SIGTSTP;
            infoReg.pidTarget = 0;
            processRegister(getpid(),SIGNAL_SENT);
            kill(getpid(), SIGTSTP); // Sends SIGTSTP signal to itself
        }
    }
}

int processRegister(pid_t pid, enum events event) {
    
    if (infoReg.hasFile == 0) return 0; 

    infoReg.file = fopen(getenv("LOG_FILENAME"), "a"); // Opens file from environmental variable, creates new if file doesnt exist yet
    char *message = malloc(MAX_BUF);
    
    // Stores current message size, needed to locate the temporary end of the string
    int messageSize = 0;

    infoReg.instant = timeElapsed(); 
    
    // Every snprintf appends to message a new string, starting from message + messageSize, while updating the new message size
    messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%f ; ", infoReg.instant);
    messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%d ; ", pid);
    
    switch (event) {
        case PROC_CREAT: // Event related to the creation of a new process
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "PROC_CREAT ; ");
            for (int c = 0; c < infoReg.numArgs; c++){
                messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%s ", infoReg.arg[c]);
            }
            snprintf(message+messageSize, MAX_BUF-messageSize, "\n");
            break;
        
        case PROC_EXIT: //Event related to the termination of a new process
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "PROC_EXIT ; ");
            snprintf(message+messageSize, MAX_BUF-messageSize, "%d\n", infoReg.exitStatus);
            break;

        case SIGNAL_RECV: //Event related to the reception of a signal by a process
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "SIGNAL_RECV ; ");
            snprintf(message+messageSize, MAX_BUF-messageSize, "%s\n", signame[infoReg.signal]);
            break;
            
        case SIGNAL_SENT: //Event related to the dispatch of a signal by a process
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "SIGNAL_SENT ; ");
            snprintf(message+messageSize, MAX_BUF-messageSize, "%s : %d\n", signame[infoReg.signal], infoReg.pidTarget);
            break;

        case FILE_MODF: //Event related to the modification of the permissions of a file/directory
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "FILE_MODF ; ");
            snprintf(message+messageSize, MAX_BUF-messageSize, "%s : 0%o : 0%o\n", infoReg.fileChanged, infoReg.oldPerm, infoReg.newPerm);
            break;
            
        default:
            break;
    }

    // Writes the whole message to log file
    fprintf(infoReg.file,"%s",message);
    
	free(message);
    fclose(infoReg.file);
    return 0;
}


int chmod_handler(char *file, mode_t newperm, mode_t oldperm) {  
    infoReg.nftot++;

    if(chmod(file, newperm) != 0) {
        perror("Error in chmod");
        return 1;
    }
    
    if (newperm != oldperm) infoReg.nfmod++;  
    
    infoReg.oldPerm = oldperm;
    infoReg.newPerm = newperm;
    processRegister(getpid(),FILE_MODF);
    return 0;
}


int directoryRecursive(char s[], char perm[], int isOctal, int option) {
    // ./xmod 0555 file.x      ./xmod u=rx file.x
    //       \/                     \/
    // perm  = "0555"           perm = "u=rx"
    // isOctal = 1              isOctal = 0
    


    DIR *dir;
    struct dirent *sd;
    char path[MAX_BUF];
    
    
    if ((dir = opendir(s)) == NULL){  // Checks if the directory opened sucessfully
        perror("Error at opening directory");
        return 1;
    }

    while ( (sd=readdir(dir)) != NULL ) { // Reads current directory into sd
        struct stat ret;
        mode_t newMode;
        
        if (strcmp(sd->d_name, ".") == 0 || strcmp(sd->d_name, "..") == 0) continue; //If the directory is of type "." or ".." it skips this process
        
        snprintf(path, sizeof(path), "%s/%s", s, sd->d_name);

        infoReg.fileChanged = path;
        if (sd->d_type == DT_DIR){    //Checks if file is a directory
            
            infoReg.arg[infoReg.numArgs-1] = path;
            processRegister(getpid(),PROC_CREAT);  

            int id = fork();
            if (id == 0) {
                execvp("./xmod", infoReg.arg);   // Calls xmod again in a deeper path. (eg. /Test1/rec -> /Test1/rec/ficheiro.txt)
                closedir(dir);
                return 0;
            } else {
                wait(NULL);  // Waits for the child process to end
            } 
        } else {            
            if (stat(path, &ret) == -1){   // The file/directory is not found
                fprintf(stderr, "Cannot access '%s': %s\n",path,strerror(errno));
                closedir(dir);
                return 1;
            }
            mode_t oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO); // Activates flags

            if (isOctal == 0) {
                if (toOctalMode(oldMode, perm, &newMode) != 0) {
                    closedir(dir);
                    return 1;
                }
            } else {
                newMode = strtol(perm, NULL, 8);  // Converts perm to octal mode
            }
            
            if(sd->d_type != DT_LNK){   // Checks if file is a symbolic link
                if (chmod_handler(path, newMode, oldMode) != 0) {
                    closedir(dir);
                    return 1;
                }
            }

            char old[10];
            char new[10];

            octalToVerb(oldMode,old);
            octalToVerb(newMode,new);
            
            switch(option) {  //Handles the "-v" and "-c" options provided by the user and prints out each corresponding output
                case V_OPTION:
                    if(sd->d_type == DT_LNK) printf("neither symbolic link '%s/'%s' nor referent has been changed\n",s,sd->d_name);
                    else if (oldMode == newMode) printf("mode of '%s/%s' retained as 0%o (%s)\n", s, sd->d_name, newMode, old);                  
                    else printf("mode of '%s/%s' changed from 0%o (%s) to 0%o (%s)\n", s, sd->d_name, oldMode, old, newMode, new);
                    break;

                case C_OPTION:
                    if (oldMode != newMode) printf("mode of '%s/%s' changed from 0%o (%s) to 0%o (%s)\n", s, sd->d_name, oldMode, old, newMode, new);
                    break;
                
                default:
                    break;
            }
        }
    }
    
    closedir(dir);
    return 0;
}

int xmod(int argc, char* argv[]) {

    int option = NO_OPTION;
    mode_t oldMode = 0;
    mode_t newMode = 0;
    int counter = 1;
    int isOctal = 1;
    struct stat ret;


    if (argc < MIN_ARGS) {
        perror("Incorrect number of arguments");
        return 1;
    }

    for (;;counter++) {
        // This infinite loop is a 5 if-else chain that iterates through the command line arguments until 
        // an invalid string, a number or a leading 'u', 'g', 'o' or 'a' is found 

        // If argument is a number, stores it in newMode in octal base 
        if ((newMode = strtol(argv[counter], NULL, 8)) != 0) {
            
            if (argv[counter][0] != '0') {
                perror("No leading zero in octal mode number");
                return 1;
            }

            if (stat(argv[counter+1], &ret) < 0) { // Tries to access file
                fprintf(stderr, "Cannot access '%s': %s\n",argv[counter+1],strerror(errno));
                return 1;
            } 
            oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO); // Activates flags
            break;
            
        } else if ( argv[counter][0] == 'u' || argv[counter][0] == 'g' || argv[counter][0] == 'o' || argv[counter][0] == 'a') {
            // Deactivates octal flag
            isOctal = 0;

            if (stat(argv[counter+1], &ret) < 0) { // Tries to access file
                fprintf(stderr, "Cannot access '%s': %s\n",argv[counter+1],strerror(errno));
                return 1;
            }
            
            oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO); // Activates flags

            if (toOctalMode(oldMode, argv[counter], &newMode) != 0) return 1;
            
            break;
        } else if (strcmp(argv[counter], "-v") == 0) {

            option &= V_OPTION_MASK; // Remove the last bit
            option |= VC_OPTION_MASK; // Indicates whether or not option -v or -c are activated

        } else if (strcmp(argv[counter], "-c") == 0) {

            option |= C_OPTION_MASK; // Sets the penultimate bit
            option |= VC_OPTION_MASK; // Indicates whether or not option -v or -c are activated

        } else if (strcmp(argv[counter], "-R") == 0) {

            option |= R_OPTION_MASK; // Sets the last bit

        } else {
            perror("Invalid arguments");
			return 1;
		}
	}

    // Another increase is needed to access the file argument
    counter++;
    
    if (chmod_handler(argv[counter], newMode, oldMode) != 0) return 1;
    
    // Creates a option copy from original without last bit, since it stored R_OPTION_MASK, which is irrelevant to verbose/changes mode checks 
    int optionCopy = option >> 1; 

    char old[10];
    char new[10];

    octalToVerb(oldMode,old);
    octalToVerb(newMode,new);

    switch(optionCopy) { //Handles the "-v" and "-c" options provided by the user and prints out each corresponding output
            case V_OPTION:
                if (oldMode == newMode) printf("mode of '%s' retained as 0%o (%s) \n", argv[counter], newMode, old);
                
                else printf("mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", argv[counter], oldMode, old, newMode, new);

                break;

            case C_OPTION:
                if (oldMode != newMode) printf("mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", argv[counter], oldMode, old, newMode, new);
                break;
            
            default:
                break;
    }

    if (option & R_OPTION_MASK) {  //If recursive flag was set 
        if(directoryRecursive(argv[counter], argv[counter - 1], isOctal, optionCopy) == 1) return 1;
    }

	return 0;
}

int main(int argc, char* argv[]) {
    char* fileName = NULL;

    infoReg.hasFile = 0;
    infoReg.numArgs = argc;
    infoReg.arg = argv;
    infoReg.invokedFile = argv[argc-1];

    char time_str[MAX_BUF];
    struct timeval start;
    
    signalSetup();

    int pid = getpid();

    if ((fileName = getenv("LOG_FILENAME")) != NULL) { // Assigns LOG_FILENAME's value to fileName if it exists

        if (getpgid(pid) == pid) {   // If it is the parent process
                                                                                           
            gettimeofday(&start,NULL);  // Stores in start the present time
            snprintf(time_str, MAX_BUF, "BEGIN_TIME=%lu %lu", start.tv_sec, start.tv_usec); // Stores in time_str the starting time
			putenv(time_str);   // Creates a env. variable called BEGIN_TIME that stores time_str
                                                                        
			if ((infoReg.file = fopen(fileName,"w") )== NULL) {  // Tries to store logFile in infoReg.file, erasing its content if it already exists
                perror("Unable to open/create log file");
                infoReg.exitStatus = EXIT_FAILURE; 
                processRegister(getpid(),PROC_EXIT);                                   
                exit(EXIT_FAILURE);
            }
            fclose(infoReg.file);
        }

        infoReg.hasFile = 1;
        infoReg.fileChanged = argv[argc-1];
    }
    
    processRegister(getpid(),PROC_CREAT);

    
    if (xmod(argc, argv) == 1){ 
        infoReg.exitStatus = EXIT_FAILURE;
        processRegister(getpid(),PROC_EXIT);
        exit(EXIT_FAILURE);  
    }

    infoReg.exitStatus = EXIT_SUCCESS;
    processRegister(getpid(),PROC_EXIT);
    
    exit(EXIT_SUCCESS);
}