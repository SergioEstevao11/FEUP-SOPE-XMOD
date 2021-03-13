#include "xmod.h"

int ViewDirectoryRecursive(char s[], int indent){
    DIR *dir;
    struct dirent *sd;
    if (!(dir = opendir(s))){
        return 1;
    }

    while ( (sd=readdir(dir)) != NULL){
        if(sd->d_type == DT_DIR){
            char path[1024];
            if(strcmp(sd->d_name, ".") == 0 || strcmp(sd->d_name, "..") == 0){
                continue;
            }
            snprintf(path, sizeof(path), "%s/%s", s, sd->d_name);
            printf("%*s[%s]\n", indent, "", sd->d_name);
            int id = fork();
            if(id == 0){
                ViewDirectoryRecursive(path, indent+2);
                return 0;
            }
            else{
                wait(NULL);
            }
        }
        else if(sd->d_type == DT_REG){
            printf(">> %s", sd->d_name);
            printf(" -> %hu\n", sd->d_reclen);
        }
    }
    closedir(dir);
    return 0;
}

int xmod(int argc, char* argv[], char* envp[]){

    if(argc < MIN_ARGS || argc > MAX_ARGS){
        fprintf(stderr,"Incorrect number of arguments\n");
        exit(EXIT_FAILURE);
    }


    if(strcmp(argv[1] , "-v")){
        
    }   
    else if(strcmp(argv[1], "-c")) {

    }
    else if(strcmp(argv[1], "-R")) {

    }


    exit(EXIT_SUCCESS);


}


int toOctalMode(mode_t oldMask, char mode[], mode_t *mask){
    mode_t newMask = 0x0;
    
    for (size_t perm = 2; perm < strlen(mode); perm++){       
        printf("Loop print: %c\n", mode[perm]);
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
    
    printf("Print 0: %o\n", newMask); 
    mode_t copyMask = newMask;

    switch(mode[0]){
        case 'u':
            newMask = newMask << 6;
            break;
        case 'g':
            newMask = newMask << 3; // newMask = 0x011000
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

    printf("Print 1: %o\n", newMask); 

    switch(mode[1]){
        case '+':
            newMask |= oldMask;
            break;
        case '-': 
            newMask = ((~newMask) & oldMask); // 0x111100111 & 0x111111111 -> 0x111100111
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
                    newMask = ((oldMask & OTHERS_MASK ) | newMask);
                    break;
                default:
                    break;
            }
            break;
        default:
            perror("Invalid Operator.\n");
            return 1;
    }

    printf("Print 2: %o\n", newMask);
    *mask = newMask;

    return 0;
}


int main(int argc, char* argv[], char* envp[]){

    struct stat ret;
    if(stat(argv[2], &ret) < 0) return 1;
    mode_t bits = ret.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

    mode_t mask;
    if(toOctalMode(bits, argv[1], &mask) != 0) exit(EXIT_FAILURE);


    

    //mode_t mode = strtol(argv[1], NULL, 8);
    if (chmod(argv[2], mask) != 0){
        fprintf(stderr, "Error in chmod\n");
    }

    return 0;
}