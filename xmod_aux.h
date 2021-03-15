#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "xmod_macros.h"

clock_t begin;

double timeElapsed();

int toOctalMode(mode_t oldMask, char mode[], mode_t *mask);
