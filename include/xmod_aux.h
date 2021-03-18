#ifndef SOPE_MP1_INCLUDE_XMOD_AUX_H_
#define SOPE_MP1_INCLUDE_XMOD_AUX_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "../include/xmod_macros.h"

/**
 * @brief Function that returns the time elapsed since the start of the execution of the program
 * 
 * @return Time elapsed
 */
double timeElapsed(void);

/**
 * @brief Function that converts an octal mode permission to a symbolic string 
 * 
 * @param perm Permition to be converted
 * @param newPerm Converted permission 
 * 
 */
void octalToVerb (mode_t perm, char * newPerm);

/**
 * @brief Function that converts a "non-octal mode" permission to an octal mode permission
 * 
 * @param oldPerm File's previous permissions
 * @param mode Non-octal mode to be converted
 * @param newPerm File's new permissions to be applied 
 * 
 * @return 0 if no errors occurred, 1 otherwise
 */
int toOctalMode(mode_t oldPerm, char mode[], mode_t *newPerm);

#endif //SOPE_MP1_INCLUDE_XMOD_AUX_H_