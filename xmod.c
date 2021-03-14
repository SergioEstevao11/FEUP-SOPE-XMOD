#include "xmod.h"

int processRegister(struct eventsInfo eevee, int fileID) { //:3
    const int MAX_BUF = 1024;
    char *message = malloc(MAX_BUF);
    int messageSize = 0;
    
    messageSize += snprintf(message, MAX_BUF, "%d ; ", eevee.instant);
    messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%d ; ", eevee.pid);
    
    switch(eevee.event){
        case PROC_CREAT:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "PROC_CREAT ; ");
            for(int c = 0; c < eevee.NumArgs; c++){
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
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%d ; ", eevee.instant);
            break;
            
        case SIGNAL_SENT:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "SIGNAL_SENT ; ");
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%d ; ", eevee.instant);
            break;

        case FILE_MODF:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "FILE_MODF ; ");
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%d ; ", eevee.instant);
            break;
            
        default:
            break;
    }
    
    free(message);
    return 0;
}



int toOctalMode(mode_t oldMask, char mode[], mode_t *mask){
    mode_t newMask = 0x0;

    for (size_t perm = 2; perm < strlen(mode); perm++) {
        switch(mode[perm]) {
            case 'r':
                newMask |= R_BIT;
                break;
            case 'w':   
                newMask |= W_BIT;
                break;

            case 'x':
                newMask |= X_BIT;
                break;

            default:
                perror("Invalid Permissions");
                return 1;
        }
    }
    
    mode_t copyMask = newMask;

    switch(mode[0]){
        case 'u':
            newMask = newMask << 6;
            break;
        case 'g':
            newMask = newMask << 3;
            break;
        case 'o':
            break;
        case 'a':
            newMask = (copyMask << 6) | (copyMask << 3) | copyMask;
            break;
        default:
            perror("Invalid User");
            return 1;
    }


    switch(mode[1]){
        case '+':
            newMask |= oldMask;
            break;
        case '-': 
            newMask = ((~newMask) & oldMask);
            break;
        case '=':
            switch (mode[0]) {
                case 'u':
                    newMask = ((oldMask & USER_MASK) | newMask);
                    break;
                case 'g':
                    newMask = ((oldMask & GROUP_MASK) | newMask);
                    break;
                case 'o':
                    newMask = ((oldMask & OTHERS_MASK) | newMask);
                    break;
                default:
                    break;
            }
            break;
        default:
            perror("Invalid Operator");
            return 1;
    }

    *mask = newMask;

    return 0;
}


int ViewDirectoryRecursive(char s[], char newMode[], int isOctal, int option){
    
    DIR *dir;
    struct dirent *sd;
    char path[1024];
    
    
    if (!(dir = opendir(s))){
        return 1;
    }

    while ( (sd=readdir(dir)) != NULL ){
        printf("%s\n", sd->d_name);
        struct stat ret;
        mode_t mask;
        char path[1024];

        if (strcmp(sd->d_name, ".") == 0 || strcmp(sd->d_name, "..") == 0){
                continue;
        }
        
        snprintf(path, sizeof(path), "%s/%s", s, sd->d_name);
        printf("%s\n", path);

        if (stat(path, &ret) == -1) return 1;
        mode_t oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
		printf("%o\n", oldMode);

		if (isOctal == 1) {
            if (toOctalMode(oldMode, newMode, &mask) != 0) exit(EXIT_FAILURE);   
        } 
                  
        else mask = strtol(newMode, NULL, 8);

        printf("%o\n", mask);
        if (chmod(path, mask) != 0) fprintf(stderr, "Error in chmod\n");
        // chamar aqui pq permissões mudaram :3
        
        switch(option) {
            case V_OPTION:
                if (oldMode == mask) printf("mode of '%s/%s' retained as %o \n", s, sd->d_name, mask); //falta dar print do mode em "rwx"
                
                else printf("mode of '%s/%s' changed from %o to %o\n", s, sd->d_name, oldMode, mask);

                break;

            case C_OPTION:
                if (oldMode != mask) printf("mode of '%s/%s' changed from %o to %o\n", s, sd->d_name, oldMode, mask);
                break;
            
            default:
                break;
        }
        
        if (sd->d_type == DT_DIR){ 

            int id = fork();
            // chamar aqui pq processo começou :3
            if (id == 0){
                ViewDirectoryRecursive(path, newMode, isOctal, option);
                return 0;
            }
            else wait(NULL);
            // chamar aqui pq processo acabou =(
        }
        
    }
    
    closedir(dir);
    return 0;
}

int xmod(int argc, char* argv[], int fileID) {

    int option = NO_OPTION;
    mode_t oldMode;
    mode_t mask;
    int counter = 1;
    int isOctal = 0;
    struct stat ret;


    if (argc < MIN_ARGS || argc > MAX_ARGS) {
        fprintf(stderr,"Incorrect number of arguments\n");
        exit(EXIT_FAILURE);
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

            if (toOctalMode(oldMode, argv[counter], &mask) != 0) exit(EXIT_FAILURE);
            
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
            exit(EXIT_FAILURE);
        }
    }


    counter++;

    
    if (chmod(argv[counter], mask) != 0) {
        fprintf(stderr, "Error in chmod\n");
        exit(EXIT_FAILURE);
    }
    else ; // chamar aqui pq permissoes mudaram :3

    int copy_option = option >> 1; 

    switch(copy_option){
            case V_OPTION:
                if (oldMode == mask) printf("mode of '%s' retained as %o \n", argv[counter], mask); //falta dar print do mode em "rwx"
                
                else printf("mode of '%s' changed from %o to %o\n", argv[counter], oldMode, mask);

                break;

            case C_OPTION:
                if (oldMode != mask) printf("mode of '%s' changed from %o to %o\n", argv[counter], oldMode, mask); //falta dar print do mode em "rwx"
                break;
            
            default:
                break;
    }

    if (option & BIT(0)) { //
        ViewDirectoryRecursive(argv[counter], argv[counter - 1], isOctal, copy_option);
    }

    exit(EXIT_SUCCESS);
}

 
int main(int argc, char* argv[], char* envp[]){
    //proc_create
    // int fid;
    // char* token;
    // char* filename = NULL;
    // FILE *file;
    
    // for (int i = 0; envp[i] != NULL; i++) { 
    //     token = strtok(envp[i], "=");
    //     if (strcmp(token,"LOG_FILENAME") == 0){
    //         filename = strtok(NULL, "=");
    //         printf("%s\n", filename);
    //         if ((fid = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
    //             perror("Unable to open/create file");
    //             exit(EXIT_FAILURE);
    //         }
    //         break;
    //     }
    // }

    // xmod(argc, argv, fid);

    // if(close(fid) == -1){
    //     perror("Unable to close file");
    //     exit(EXIT_FAILURE);
    // };

    int sig = 2;

    //char *str = strdup(sys_siglist[sig]);
    char *str = strsignal(sig);

    //char *signame[]={"INVALID", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1", "SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM", "SIGSTKFLT", "SIGCHLD", "SIGCONT", "SIGSTOP", "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGPOLL", "SIGPWR", "SIGSYS", NULL};
    // while (*str)
    // {
    //     *str = toupper(*str);
    //     str++;
    // }

    printf("%2d -> SIG%s\n", sig, str);
    return 0;
}