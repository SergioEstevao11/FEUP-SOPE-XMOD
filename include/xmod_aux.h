#ifndef SOPE_MP1_INCLUDE_XMOD_AUX_H_
#define SOPE_MP1_INCLUDE_XMOD_AUX_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "../include/xmod_macros.h"

double timeElapsed(void);

void octalToVerb (mode_t perm, char * mode);

int toOctalMode(mode_t oldMask, char mode[], mode_t *mask);

#endif //SOPE_MP1_INCLUDE_XMOD_AUX_H_