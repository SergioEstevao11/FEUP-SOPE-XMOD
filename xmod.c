#include "xmod.h"

void sig_handler(int signal) { // :3
    char* x;
    eevee.signal = signal;
    processRegister(SIGNAL_RECV);
    if (signal == SIGINT) {
        // if(getpgid(getpid()) != getpid()){ // Faz com q todos menos o processo master parem quietos
        //     kill(getpid(), SIGTSTP);
        //     eevee.signal = SIGTSTP;
        //     processRegister();
        //     // Escrever o sinal no ficheiro de quem manda o sinal
        //     // Enviar o sinal para o processo desejado
        //     // Escrever o sinal no ficheiro de quem recebe o sinal (atraves do sig_handler)
        // }
        if(getpgid(getpid()) == getpid()) { // Se for o processo master
            printf("\n%d ; %s ; %d ; %d\n", getpid(), eevee.fileChanged, eevee.nftot, eevee.nfmod);

            do {
                printf("Do you want to terminate the program? (y/n) ");
                scanf("%s", x);
                while ((getchar()) != '\n'); 

            } while ( sizeof(x) / sizeof(char) != 1 && x[0] != 'n' && x[0] != 'y' );
            
            if (x[0] == 'y') {
                kill(0,SIGKILL);
                eevee.signal = SIGKILL;
                eevee.pid = getpid();
                eevee.pidTarget = 0;
                processRegister(SIGNAL_SENT);
            }
            else {
                kill(0,SIGCONT);
                eevee.signal = SIGCONT;
                eevee.pid = getpid();
                eevee.pidTarget = 0;
                processRegister(SIGNAL_SENT);
            }
        }
    }
}

int chmod_handler(char *file, mode_t newperm, mode_t oldperm){
    eevee.nftot++;
    if(chmod(file, newperm) != 0){
        return 1;
    }
    if (newperm != oldperm){
        eevee.nfmod;
    }
    
    eevee.oldPerm = oldperm;
    eevee.newPerm = newperm;
    processRegister(FILE_MODF);
    return 0;
}

int processRegister(enum events event) { // :3
    if (eevee.hasFile == 0){
        printf("Eevee has no file :(\n");
        return 0;
    }
    eevee.file = fopen(getenv("LOG_FILENAME"), "a");
    const int MAX_BUF = 1024;
    char *message = malloc(MAX_BUF);
    int messageSize = 0;

    eevee.pid = getpid();
    eevee.instant = timeElapsed();
    
    messageSize += snprintf(message, MAX_BUF, "%f ; ", eevee.instant);
    messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "%d ; ", eevee.pid);

    
    switch(event){
        case PROC_CREAT:
            messageSize += snprintf(message+messageSize, MAX_BUF-messageSize, "PROC_CREAT ; ");
            for (int c = 0; c < eevee.NumArgs; c++){
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
            break;
            
        default:
            break;
    }

    //printf("%s\n", message);

    fprintf(eevee.file,"%s",message);
    
	free(message);
    fclose(eevee.file);
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
        //printf("%s\n", sd->d_name);
        struct stat ret;
        mode_t mask;
        char path[1024];
        
        if (strcmp(sd->d_name, ".") == 0 || strcmp(sd->d_name, "..") == 0){
                continue;
        }

        
        snprintf(path, sizeof(path), "%s/%s", s, sd->d_name);

        eevee.fileChanged = path;
        if (sd->d_type == DT_DIR){ 

            int id = fork();
            // chamar aqui pq processo começou :3
            if (id == 0) {
                eevee.arg[eevee.NumArgs-1] = path;

                execvp("./xmod", eevee.arg);
                return 0;
            }
            else wait(NULL);
            // chamar aqui pq processo acabou =( que fica ficou quem foi vai vai vai
        }
        else{
            //eliminar daqui
            // obrigado rei da informação por nos abençoares com este comentário util pra caralho :pray:

            if (stat(path, &ret) == -1) return 1;
            mode_t oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

            if (isOctal == 1) {
                if (toOctalMode(oldMode, newMode, &mask) != 0) exit(EXIT_FAILURE);   
            } 
                    
            else mask = strtol(newMode, NULL, 8);

            if (chmod_handler(path, mask, oldMode) != 0) {
                return 1;	
            }
            
            
            // chamar aqui pq permissões mudaram :3
            
            switch(option) {
                case V_OPTION:
                    if (oldMode == mask) printf("mode of '%s/%s' retained as 0%o \n", s, sd->d_name, mask); //falta dar print do mode em "rwx"                    
                    else printf("mode of '%s/%s' changed from 0%o to 0%o\n", s, sd->d_name, oldMode, mask);
                    break;

                case C_OPTION:
                    if (oldMode != mask) printf("mode of '%s/%s' changed from 0%o to 0%o\n", s, sd->d_name, oldMode, mask);
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
        processRegister(PROC_EXIT);
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

            if (toOctalMode(oldMode, argv[counter], &mask) != 0){
                eevee.exitStatus = EXIT_FAILURE;
                processRegister(PROC_EXIT);
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
            processRegister(PROC_EXIT);
			return 1;
		}
	}

    counter++;

    
    if (chmod_handler(argv[counter], mask, oldMode) != 0) {
		return 1;
	}
    
	// chamar aqui pq permissoes mudaram :3

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

    if (option & BIT(0)) { 
        ViewDirectoryRecursive(argv[counter], argv[counter - 1], isOctal, copy_option);
    }

	return 0;
}

int signalSetup(){
    struct sigaction sig, old_action;  

    sigemptyset(&sig.sa_mask);          
    sig.sa_flags = 0;                   
    sig.sa_handler = sig_handler; 

    if(sigaction(SIGINT, &sig, &old_action)){
        perror("Error in sigaction");
        return 1;
    }
    if(sigaction(SIGKILL, &sig, &old_action)){
        perror("Error in sigaction");
        return 1;
    }
    if(sigaction(SIGHUP, &sig, &old_action)){
        perror("Error in sigaction");
        return 1;
    }
    if(sigaction(SIGUSR1, &sig, &old_action)){
        perror("Error in sigaction");
        return 1;
    }
    if(sigaction(SIGQUIT, &sig, &old_action)){
        perror("Error in sigaction");
        return 1;
    }
    if(sigaction(SIGSEGV, &sig, &old_action)){
        perror("Error in sigaction");
        return 1;
    }
    if(sigaction(SIGUSR2, &sig, &old_action)){
        perror("Error in sigaction");
        return 1;
    }
    if(sigaction(SIGPIPE, &sig, &old_action)){
        perror("Error in sigaction");
        return 1;
    }
    if(sigaction(SIGALRM, &sig, &old_action)){
        perror("Error in sigaction");
        return 1;
    }
    if(sigaction(SIGTERM, &sig, &old_action)){
        perror("Error in sigaction");
        return 1;
    }
    if(sigaction(SIGCHLD, &sig, &old_action)){
        perror("Error in sigaction");
        return 1;
    }

    return 0;

}

int main(int argc, char* argv[], char* envp[]){
    int fid;
    char* filename = NULL;

    eevee.hasFile = 0;
    eevee.arg = argv;
    eevee.NumArgs = argc;

    char time_str[1024];
    struct timeval start;

    signalSetup();

    int pid = getpid();

    if ((filename = getenv("LOG_FILENAME"))!= NULL) { // nao deteta
        if (getpgid(pid) == pid){
            gettimeofday(&start,NULL);
            snprintf(time_str, 1024, "BEGIN_TIME=%lu %lu", start.tv_sec, start.tv_usec);
			putenv(time_str);
			if ((eevee.file = fopen(filename,"w") )== NULL) {
                perror("Unable to open/create file");
                eevee.exitStatus = EXIT_FAILURE;
                processRegister(PROC_EXIT);
                exit(EXIT_FAILURE);
            }
            fclose(eevee.file);
        }
        eevee.hasFile = 1;
        eevee.fileChanged = argv[argc-1];
    }

    
    if (xmod(argc, argv) == 1){ 
        exit(EXIT_FAILURE);  
    }


    if (eevee.hasFile == 1) {
        eevee.exitStatus = EXIT_SUCCESS;
        processRegister(PROC_EXIT);
    }
    
    exit(EXIT_SUCCESS);
}