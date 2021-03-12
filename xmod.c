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
                ViewDirectory(path, indent+2);
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
    if(argv[1] == "-v"){
        //tratar v
    }
    else if(argv[1] == "-c")

}
