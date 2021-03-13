#include "xmod.h"


int toOctalMode(mode_t oldMask, char mode[], mode_t *mask){
    mode_t newMask = 0x0;
    
    for (size_t perm = 2; perm < strlen(mode); perm++){       
        switch(mode[perm]){
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
                perror("Invalid Permissions.\n");
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
            perror("Invalid User.\n");
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
            perror("Invalid Operator.\n");
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
        struct stat ret;
        mode_t mask;
        char path[1024];
        
        snprintf(path, sizeof(path), "%s/%s", s, sd->d_name);

        if (stat(path, &ret) < 0) return 1;
        mode_t oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
        
        if (isOctal) 
            if (toOctalMode(oldMode, newMode, &mask) != 0) exit(EXIT_FAILURE);    
                  
        else mask = strtol(newMode, NULL, 8);

        if (chmod(path, mask) != 0) fprintf(stderr, "Error in chmod\n");
        
        switch(option){
            case V_OPTION: //option v
                if (oldMode == mask) printf("mode of '%s/%s' retained as %o \n", s, sd->d_name, mask); //falta dar print do mode em "rwx"
                
                else printf("mode of '%s/%s' changed from %o to %o\n", s, sd->d_name, oldMode, mask);

                break;

            case C_OPTION:// option c
                if (oldMode != mask) printf("mode of '%s/%s' changed from %o to %o\n", s, sd->d_name, oldMode, mask);
                break;
            
            default:
                break;
        }
        
        if (sd->d_type == DT_DIR){ 

            if (strcmp(sd->d_name, ".") == 0 || strcmp(sd->d_name, "..") == 0){
                continue;
            }
            else{
                int id = fork();
            
                if (id == 0){
                    ViewDirectoryRecursive(path, newMode, isOctal, option);
                    return 0;
                }
                else wait(NULL);
            }
        }
        
        // else if(sd->d_type == DT_REG){
    
        // }
    }
    
    closedir(dir);
    return 0;
}

int xmod(int argc, char* argv[], char* envp[]) {

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

            if (stat(argv[counter+1], &ret) < 0) return 1;
            oldMode = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

            break;
        }

        else if ( argv[counter][0] == 'u' || argv[counter][0] == 'g' || argv[counter][0] == 'o' || argv[counter][0] == 'a'){
            isOctal = 1;

            if (stat(argv[counter+1], &ret) < 0) return 1;
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

    
    if (chmod(argv[counter], mask) != 0){
        fprintf(stderr, "Error in chmod\n");
        exit(EXIT_FAILURE);
    }

    int copy_option = option >> 1; 

    switch(copy_option){
            case V_OPTION: //option v
                if (oldMode == mask) printf("mode of '%s' retained as %o \n", argv[counter], mask); //falta dar print do mode em "rwx"
                
                else printf("mode of '%s' changed from %o to %o\n", argv[counter], oldMode, mask);

                break;

            case C_OPTION:// option c
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
    xmod(argc, argv, envp);
    return 0;
}