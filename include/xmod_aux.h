#ifndef XMOD_AUX_H
#define XMOD_AUX_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "xmod_macros.h"

double timeElapsed(void);

void octalToVerb (mode_t perm, char * mode);

int toOctalMode(mode_t oldMask, char mode[], mode_t *mask);

#endif