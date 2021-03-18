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
        if (getpgid(getpid()) == getpid()) { // 
            printf("\n%d ; %s ; %d ; %d\n", getpid(), infoReg.fileChanged, infoReg.nftot, infoReg.nfmod);

            do {
                printf("Do you want to terminate the program? (y/n) ");
                scanf("%s", input);
                while ((getchar()) != '\n'); 

            } while ( strcmp(input, "y") != 0 && strcmp(input, "n") != 0 );
            if (input[0] == 'y') {
                infoReg.signal = SIGKILL;
                infoReg.pidTarget = 0;
                processRegister(getpid(),SIGNAL_SENT);
                kill(0,SIGKILL);
            } else {
                infoReg.signal = SIGCONT;
                infoReg.pidTarget = 0;
                processRegister(getpid(),SIGNAL_SENT);
                kill(0,SIGCONT);
            }
        } else {
            infoReg.signal = SIGTSTP;
            infoReg.pidTarget = 0;
            processRegister(getpid(),SIGNAL_SENT);
            kill(getpid(), SIGTSTP);
        }
    }
}

int processRegister(pid_t pid, enum events event) {
    //
    if (infoReg.hasFile == 0) return 1; 

    infoReg.file = fopen(getenv("LOG_FILENAME"), "a");
    char *message = malloc(MAX_BUF);
    int messageSize = 0;

    infoReg.instant = timeElapsed();
    
    messageSize += snprintf(message, MAX_BUF, "%f ; ", infoReg.instant);
    messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%d ; ", pid);
    
    switch (event) {
        case PROC_CREAT:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "PROC_CREAT ; ");
            for (int c = 0; c < infoReg.numArgs; c++){
                messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%s ", infoReg.arg[c]);
            }
            snprintf(message+messageSize, MAX_BUF-messageSize, "\n");
            break;
        
        case PROC_EXIT:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "PROC_EXIT ; ");
            snprintf(message+messageSize, MAX_BUF-messageSize, "%d\n", infoReg.exitStatus);
            break;

        case SIGNAL_RECV:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "SIGNAL_RECV ; ");
            snprintf(message+messageSize, MAX_BUF-messageSize, "%s\n", signame[infoReg.signal]);
            break;
            
        case SIGNAL_SENT:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "SIGNAL_SENT ; ");
            snprintf(message+messageSize, MAX_BUF-messageSize, "%s : %d\n", signame[infoReg.signal], infoReg.pidTarget);
            break;

        case FILE_MODF:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "FILE_MODF ; ");
            snprintf(message+messageSize, MAX_BUF-messageSize, "%s : 0%o : 0%o\n", infoReg.fileChanged, infoReg.oldPerm, infoReg.newPerm);
            break;
            
        default:
            break;
    }

    fprintf(infoReg.file,"%s",message);
    
	free(message);
    fclose(infoReg.file);
    return 0;
}


int chmod_handler(char *file, mode_t newperm, mode_t oldperm){
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


int viewDirectoryRecursive(char s[], char newMode[], int isOctal, int option){
    
    DIR *dir;
    struct dirent *sd;
    char path[MAX_BUF];
    
    
    if ((dir = opendir(s)) == NULL){
        perror("Error at opening directory");
        return 1;
    }

    while ( (sd=readdir(dir)) != NULL ) {
        struct stat ret;
        mode_t mask;
        
        if (strcmp(sd->d_name, ".") == 0 || strcmp(sd->d_name, "..") == 0) continue;
        
        snprintf(path, sizeof(path), "%s/%s", s, sd->d_name);

        infoReg.fileChanged = path;
        if (sd->d_type == DT_DIR){ 
            
            infoReg.arg[infoReg.numArgs-1] = path;
            processRegister(getpid(),PROC_CREAT);

            int id = fork();
            if (id == 0) {
                execvp("./xmod", infoReg.arg);
                closedir(dir);
                return 0;
            } else {
                wait(NULL);
            } 
        } else {

            if (stat(path, &ret) == -1){
                closedir(dir);
                return 1;
            }
            mode_t oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

            if (isOctal == 1) {
                if (toOctalMode(oldMode, newMode, &mask) != 0){
                    closedir(dir);
                    return 1;
                }
            } 
                    
            else mask = strtol(newMode, NULL, 8);

            if(sd->d_type != DT_LNK){
                if (chmod_handler(path, mask, oldMode) != 0) {
                    closedir(dir);
                    return 1;
                }
            }

            char old[10];
            char new[10];

            octalToVerb(oldMode,old);
            octalToVerb(mask,new);
            
            switch(option) {
                case V_OPTION:
                    if(sd->d_type == DT_LNK) printf("neither symbolic link '%s/'%s' nor referent has been changed\n",s,sd->d_name);
                    else if (oldMode == mask) printf("mode of '%s/%s' retained as 0%o (%s)\n", s, sd->d_name, mask, old); //falta dar print do mode em "rwx"                    
                    else printf("mode of '%s/%s' changed from 0%o (%s) to 0%o (%s)\n", s, sd->d_name, oldMode, old, mask, new);
                    break;

                case C_OPTION:
                    if (oldMode != mask) printf("mode of '%s/%s' changed from 0%o (%s) to 0%o (%s)\n", s, sd->d_name, oldMode, old, mask, new);
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
    int isOctal = 0;
    struct stat ret;


    if (argc < MIN_ARGS) {
        perror("Incorrect number of arguments");
        return 1;
    }


    for (;;counter++) {
        
        if ((newMode = strtol(argv[counter], NULL, 8)) != 0) {
            
            if (argv[counter][0] != '0'){
                perror("No leading zero in octal mode number");
                return 1;
            }

            if (stat(argv[counter+1], &ret) < 0){
                fprintf(stderr, "Cannot access '%s': %s\n",argv[counter+1],strerror(errno));
                return 1;
            } 
            oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

            break;
        } else if ( argv[counter][0] == 'u' || argv[counter][0] == 'g' || argv[counter][0] == 'o' || argv[counter][0] == 'a'){
            isOctal = 1;

            if (stat(argv[counter+1], &ret) < 0) {
                fprintf(stderr, "Cannot access '%s': %s\n",argv[counter+1],strerror(errno));
                return 1;
            }
            oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

            if (toOctalMode(oldMode, argv[counter], &newMode) != 0) {
                return 1;
            }
            
            break;
        } else if (strcmp(argv[counter], "-v") == 0) {

            option &= V_OPTION_MASK; // Tira o penultimo bit 
            option |= VC_OPTION_MASK; // Sinaliza q existe opçao -v/-c

        } else if (strcmp(argv[counter], "-c") == 0) {

            option |= C_OPTION_MASK; // Mete o penultimo bit
            option |= VC_OPTION_MASK; // Sinaliza q existe opção -v/-c

        } else if (strcmp(argv[counter], "-R") == 0) {

            option |= R_OPTION_MASK; // Mete o ultimo bit

        } else {
            perror("Invalid argument after options");
			return 1;
		}
	}

    counter++;
    
    if (chmod_handler(argv[counter], newMode, oldMode) != 0) return 1;
    
    int copy_option = option >> 1; 

    char old[10];
    char new[10];

    octalToVerb(oldMode,old);
    octalToVerb(newMode,new);

    switch(copy_option){
            case V_OPTION:
                if (oldMode == newMode) printf("mode of '%s' retained as 0%o (%s) \n", argv[counter], newMode, old); //falta dar print do mode em "rwx"
                
                else printf("mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", argv[counter], oldMode, old, newMode, new);

                break;

            case C_OPTION:
                if (oldMode != newMode) printf("mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", argv[counter], oldMode, old, newMode, new); //falta dar print do mode em "rwx"
                break;
            
            default:
                break;
    }

    if (option & R_OPTION_MASK) { 
        viewDirectoryRecursive(argv[counter], argv[counter - 1], isOctal, copy_option);
    }

	return 0;
}

int main(int argc, char* argv[]) {
    char* filename = NULL;

    infoReg.hasFile = 0;
    infoReg.numArgs = argc;
    infoReg.arg = argv;

    char time_str[MAX_BUF];
    struct timeval start;
    
    signalSetup();

    int pid = getpid();

    if ((filename = getenv("LOG_FILENAME")) != NULL) { // Se LOG_FILENAME existir

        if (getpgid(pid) == pid) {                                                          // Se o processo atual for o principal
                                                                                           
            gettimeofday(&start,NULL);                                                      // Guarda em start o tempo atual para cronometrar cada processo
            snprintf(time_str, MAX_BUF, "BEGIN_TIME=%lu %lu", start.tv_sec, start.tv_usec);
			putenv(time_str);                                                               // Cria uma env. var. chamada BEGIN_TIME q guarda o tempo existente em start 
                                                                        
			if ((infoReg.file = fopen(filename,"w") )== NULL) {                              // Tenta guardar em infoReg.file o logFile, se n der:
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

    sleep(5);
    
    if (xmod(argc, argv) == 1){ 
        infoReg.exitStatus = EXIT_FAILURE;
        processRegister(getpid(),PROC_EXIT);
        exit(EXIT_FAILURE);  
    }

    infoReg.exitStatus = EXIT_SUCCESS;
    processRegister(getpid(),PROC_EXIT);
    
    exit(EXIT_SUCCESS);
}