#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "xmod_macros.h"

double timeElapsed();

int toOctalMode(mode_t oldMask, char mode[], mode_t *mask);
