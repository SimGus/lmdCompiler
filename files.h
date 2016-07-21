#ifndef _FILENAME_H_
#define _FILENAME_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "error.h"

#define TMP_OUTPUT_FILENAME "LMD-%s.tmp"

#define RAND_INT(min, max)	(rand()%(max+1-min))+min

char* getDirName(const char* filePath);
char* getBaseName(const char* filePath);

char* getOutputNameFromInputName(const char* inputFileName);

char* addLmdExtension(const char* fileName);
char* addTexExtension(const char* fileName);
char* addPdfExtension(const char* fileName);

char* getRandomStringOf10Chars();

/*
 * @return :   a string containing the name of the temporary file
 * @post : the string returned MUST be freed
 */
char* getTmpFileName();

/*
 * Deletes the file with path filePath
 * @return :   RETURN_SUCCESS if everything worked fine
 *             RETURN_FAILURE if the file couldn't be deleted
 */
STATUS deleteFile(const char* filePath);

/*
 * @return :   true if the file with path filePath exists
 *             false otherwise
 */
bool fileExists(const char* filePath);

#endif //_FILENAME_H_
