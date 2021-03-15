#include "xmod_aux.h"

double timeElapsed(){
    //struct timeval midTime;
    clock_t midTime = clock();
    double time_spent =((double)(midTime-begin) / CLOCKS_PER_SEC)*1000;
	return time_spent;
}

int toOctalMode(mode_t oldMask, char mode[], mode_t *mask){
    mode_t newMask = 0x0;
    int rFlag = 0, wFlag = 0, xFlag = 0;

    for (size_t perm = 2; perm < strlen(mode); perm++) {
        switch (mode[perm]) {
            case 'r':
                if (!rFlag) {
                    newMask |= R_BIT;
                    rFlag = 1;
                    break;
                } else {
                    perror("Invalid arguments");
                    return 1;
                }
            case 'w':
                if (!wFlag) {
                    newMask |= W_BIT;
                    rFlag = 1;
                    wFlag = 1;
                    break;
                } else {
                    perror("Invalid arguments");
                    return 1;
                }
            case 'x':
                if (!xFlag) {
                    newMask |= X_BIT;
                    rFlag = 1;
                    wFlag = 1;
                    xFlag = 1;
                    break;
                } else {
                    perror("Invalid arguments");
                    return 1;
                }
            default:
                perror("Invalid Permissions");
                return 1;
        }
    }
    
    mode_t copyMask = newMask;

    switch (mode[0]) {
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


    switch (mode[1]) {
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
