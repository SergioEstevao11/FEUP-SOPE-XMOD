#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "xmod_macros.h"

double timeElapsed();

const char* octalToVerb (mode_t perm);

int toOctalMode(mode_t oldMask, char mode[], mode_t *mask);
