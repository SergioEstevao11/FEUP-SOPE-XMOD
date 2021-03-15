#include "xmod.h"

void sig_handler(int signal) { // :3
    eevee.signal = signal;
    processRegister(SIGNAL_RECV);
    if (signal == SIGINT) {
        char x;
        printf("%d ; %s ; %d ; %d\n", getpid(), eevee.fileChanged, eevee.nftot, eevee.nfmod);
        printf("Do you want to terminate the program? (y/n) ");
        scanf("%c", &x);
        if (x == 'y') {
            eevee.exitStatus = EXIT_SUCCESS;
            processRegister(PROC_EXIT);
            exit(EXIT_SUCCESS);
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
    if(eevee.hasFile == 0){
        return 0;
    }
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

    printf("%s\n", message);

    fprintf(eevee.file,"%s",message);
    
	free(message);
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
        printf("%s\n", path);

        eevee.fileChanged = path;
        if (sd->d_type == DT_DIR){ 

            int id = fork();
            // chamar aqui pq processo começou :3
            
            if (id == 0){
                //fclose(eevee.file);
                //fflush(eevee.file);
                eevee.arg[eevee.NumArgs-1] = path;
                for(int c = 0; c < eevee.NumArgs; c++){
                    printf("-> %s \n", eevee.arg[c]);
                }
                execvp("./xmod", eevee.arg);
                //execvp("./xmod", argumentos da main)
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
            printf("%o\n", oldMode);

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
                    if (oldMode == mask) printf("mode of '%s/%s' retained as %o \n", s, sd->d_name, mask); //falta dar print do mode em "rwx"
                    
                    else printf("mode of '%s/%s' changed from %o to %o\n", s, sd->d_name, oldMode, mask);

                    break;

                case C_OPTION:
                    if (oldMode != mask) printf("mode of '%s/%s' changed from %o to %o\n", s, sd->d_name, oldMode, mask);
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

int main(int argc, char* argv[], char* envp[]){
    begin = clock();

    int fid;
    char* token;
    char* filename = NULL;

    eevee.hasFile = 0;
    eevee.arg = argv;
    eevee.NumArgs = argc;

    if ((filename = getenv("LOG_FILENAME"))!= NULL) {
        if ((eevee.file = fopen(filename,"w") )== NULL) {
            perror("Unable to open/create file");
            eevee.exitStatus = EXIT_FAILURE;
            processRegister(PROC_EXIT);
            exit(EXIT_FAILURE);
        }

        eevee.hasFile = 1;
        eevee.fileChanged = argv[argc-1];

    }
	/*
    struct sigaction sig;
    sig*/

    //signal(NSIG, sig_handler);
    
    if (xmod(argc, argv) == 1){ 
        if(eevee.hasFile == 1) fclose(eevee.file);
        exit(EXIT_FAILURE);  
    }

    if (eevee.hasFile == 1) {
        eevee.exitStatus = EXIT_SUCCESS;
        processRegister(PROC_EXIT);
        fclose(eevee.file);
    }
    
    exit(EXIT_SUCCESS);
}