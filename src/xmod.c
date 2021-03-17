#include "../include/xmod.h"

void sig_handler(int signal) { // :3
    char *input = malloc(MAX_BUF);

    eevee.signal = signal;
    processRegister(getpid(), SIGNAL_RECV);
    if (signal == SIGINT) {
        if (getpgid(getpid()) == getpid()) { // Se for o processo master
            printf("\n%d ; %s ; %d ; %d\n", getpid(), eevee.fileChanged, eevee.nftot, eevee.nfmod);

            do {
                printf("Do you want to terminate the program? (y/n) ");
                scanf("%s", input);
                while ((getchar()) != '\n'); 

            } while ( /*sizeof(x) / sizeof(char)*/ strlen(input) != 1 && input[0] != 'n' && input[0] != 'y' );
            
            if (input[0] == 'y') {
                eevee.signal = SIGKILL;
                eevee.pidTarget = 0;
                processRegister(getpid(),SIGNAL_SENT);
                free(input);
                kill(0,SIGKILL);
            }
            else {
                eevee.signal = SIGCONT;
                eevee.pidTarget = 0;
                processRegister(getpid(),SIGNAL_SENT);
                kill(0,SIGCONT);
            }
        }
        else {
            eevee.signal = SIGTSTP;
            eevee.pidTarget = 0;
            processRegister(getpid(),SIGNAL_SENT);
            kill(getpid(), SIGTSTP);
        }
    }
    free(input);
}

int chmod_handler(char *file, mode_t newperm, mode_t oldperm){
    eevee.nftot++;

    if(chmod(file, newperm) != 0) return 1;
    if (newperm != oldperm) eevee.nfmod++;  
    
    eevee.oldPerm = oldperm;
    eevee.newPerm = newperm;
    processRegister(getpid(),FILE_MODF);
    return 0;
}

int processRegister(pid_t pid, enum events event) {
    if (eevee.hasFile == 0) return 1;

    eevee.file = fopen(getenv("LOG_FILENAME"), "a");
    char *message = malloc(MAX_BUF);
    int messageSize = 0;

    eevee.instant = timeElapsed();
    
    messageSize += snprintf(message, MAX_BUF, "%f ; ", eevee.instant);
    messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%d ; ", pid);
    
    switch (event) {
        case PROC_CREAT:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "PROC_CREAT ; ");
            for (int c = 0; c < eevee.numArgs; c++){
                messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%s ", eevee.arg[c]);
            }
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "\n");
            break;
        
        case PROC_EXIT:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "PROC_EXIT ; ");
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%d\n", eevee.exitStatus);
            break;

        case SIGNAL_RECV:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "SIGNAL_RECV ; ");
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%s\n", signame[eevee.signal]);
            break;
            
        case SIGNAL_SENT:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "SIGNAL_SENT ; ");
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%s : %d\n", signame[eevee.signal], eevee.pidTarget);
            break;

        case FILE_MODF:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "FILE_MODF ; ");
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%s : 0%o : 0%o\n", eevee.fileChanged, eevee.oldPerm, eevee.newPerm);
            break;;
            break;
            
        default:
            break;
    }

    fprintf(eevee.file,"%s",message);
    
	free(message);
    fclose(eevee.file);
    return 0;
}


int viewDirectoryRecursive(char s[], char newMode[], int isOctal, int option){
    
    DIR *dir;
    struct dirent *sd;
    char path[MAX_BUF];
    
    
    if ((dir = opendir(s)) == NULL){
        printf("Error at opening directory");
        return 1;
    }

    while ( (sd=readdir(dir)) != NULL ) {
        //printf("%s\n", sd->d_name);
        struct stat ret;
        mode_t mask;
        
        if (strcmp(sd->d_name, ".") == 0 || strcmp(sd->d_name, "..") == 0) continue;
        
        snprintf(path, sizeof(path), "%s/%s", s, sd->d_name);

        eevee.fileChanged = path;
        if (sd->d_type == DT_DIR){ 
            
            eevee.arg[eevee.numArgs-1] = path;
            processRegister(getpid(),PROC_CREAT);

            int id = fork();
            if (id == 0) {
                execvp("./xmod", eevee.arg);
                return 0;
            } else {
                wait(NULL);
            } 
        } else {

            if (stat(path, &ret) == -1) return 1;
            mode_t oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

            if (isOctal == 1) {
                if (toOctalMode(oldMode, newMode, &mask) != 0) return 1;
            } 
                    
            else mask = strtol(newMode, NULL, 8);

            if(sd->d_type != DT_LNK)
                if (chmod_handler(path, mask, oldMode) != 0) return 1;
            
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
    mode_t oldMode;
    mode_t mask;
    int counter = 1;
    int isOctal = 0;
    struct stat ret;


    if (argc < MIN_ARGS || argc > MAX_ARGS) {
        perror("Incorrect number of arguments");
        eevee.exitStatus = EXIT_FAILURE;
        processRegister(getpid(),PROC_EXIT);
        return 1;
    }


    for (;;counter++) {
        
        if ((mask = strtol(argv[counter], NULL, 8)) != 0) {
            
            if (argv[counter][0] != '0') return 1;

            if (stat(argv[counter+1], &ret) < 0) return 1; // Possivelmente vai ser preciso fazer um for aqui para alterar varios ficheiros
            oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

            break;
        }

        else if ( argv[counter][0] == 'u' || argv[counter][0] == 'g' || argv[counter][0] == 'o' || argv[counter][0] == 'a'){
            isOctal = 1;

            if (stat(argv[counter+1], &ret) < 0) return 1; // Possivelmente vai ser preciso fazer um for aqui para alterar varios ficheiros
            oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

            if (toOctalMode(oldMode, argv[counter], &mask) != 0) {
                eevee.exitStatus = EXIT_FAILURE;
                processRegister(getpid(),PROC_EXIT);
                return 1;
            }
            
            break;
        }

    
        else if (strcmp(argv[counter], "-v") == 0) {
            option &= V_OPTION_MASK; // Tira o penultimo bit 
            option |= VC_OPTION_MASK; // Sinaliza q existe opçao -v/-c
        }

        

        else if (strcmp(argv[counter], "-c") == 0) {
            option |= C_OPTION_MASK; // Mete o penultimo bit
            option |= VC_OPTION_MASK; // Sinaliza q existe opção -v/-c
        }

        else if (strcmp(argv[counter], "-R") == 0) option |= R_OPTION_MASK; // Mete o ultimo bit

        else {
            perror("Invalid argument after options");
            eevee.exitStatus = EXIT_FAILURE;
            processRegister(getpid(),PROC_EXIT);
			return 1;
		}
	}

    counter++;
    
    if (chmod_handler(argv[counter], mask, oldMode) != 0) return 1;
    
    int copy_option = option >> 1; 

    char old[10];
    char new[10];

    octalToVerb(oldMode,old);
    octalToVerb(mask,new);

    switch(copy_option){
            case V_OPTION:
                if (oldMode == mask) printf("mode of '%s' retained as 0%o (%s) \n", argv[counter], mask, old); //falta dar print do mode em "rwx"
                
                else printf("mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", argv[counter], oldMode, old, mask, new);

                break;

            case C_OPTION:
                if (oldMode != mask) printf("mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", argv[counter], oldMode, old, mask, new); //falta dar print do mode em "rwx"
                break;
            
            default:
                break;
    }

    if (option & R_OPTION_MASK) { 
        viewDirectoryRecursive(argv[counter], argv[counter - 1], isOctal, copy_option);
    }

	return 0;
}

int signalSetup(){
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
    /*if (sigaction(SIGSEGV, &sig, &old_action) == -1){
        perror("Error in sigaction, SIGSEGV");
        return 1;
    }
    */
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

int main(int argc, char* argv[]) {
    char* filename = NULL;

    eevee.hasFile = 0;
    eevee.numArgs = argc;
    eevee.arg = argv;

    char time_str[MAX_BUF];
    struct timeval start;
    
    signalSetup();

    int pid = getpid();

    if ((filename = getenv("LOG_FILENAME")) != NULL) { // Se LOG_FILENAME existir

        if (getpgid(pid) == pid) {                                                          // Se o processo atual for o principal
                                                                                           
            gettimeofday(&start,NULL);                                                      // Guarda em start o tempo atual para cronometrar cada processo
            snprintf(time_str, MAX_BUF, "BEGIN_TIME=%lu %lu", start.tv_sec, start.tv_usec);
			putenv(time_str);                                                               // Cria uma env. var. chamada BEGIN_TIME q guarda o tempo existente em start 
                                                                        
			if ((eevee.file = fopen(filename,"w") )== NULL) {                              // Tenta guardar em eevee.file o logFile, se n der:
                perror("Unable to open/create log file");
                eevee.exitStatus = EXIT_FAILURE; 
                processRegister(getpid(),PROC_EXIT);                                   
                exit(EXIT_FAILURE);
            }
            fclose(eevee.file);
        }

        eevee.hasFile = 1;
        eevee.fileChanged = argv[argc-1];
    }
    
    processRegister(getpid(),PROC_CREAT);

    //sleep(5);
    
    if (xmod(argc, argv) == 1){ 
        eevee.exitStatus = EXIT_FAILURE;
        processRegister(getpid(),PROC_EXIT);
        exit(EXIT_FAILURE);  
    }

    eevee.exitStatus = EXIT_SUCCESS;
    processRegister(getpid(),PROC_EXIT);
    
    exit(EXIT_SUCCESS);
}