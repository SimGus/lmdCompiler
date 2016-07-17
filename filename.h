#ifndef _FILENAME_H_
#define _FILENAME_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"

typedef enum
{
   TEX, PDF, BOTH
} OutputType;

char* argToInputFileName(const char* inputArg);
char* argToOutputFileName(const char* outputArg, OutputType fileType);
void replaceExtension(char* string, OutputType fileType);

char* getFileNameWithoutExtension(const char* fileName);

char* getDirName(const char* filePath);
char* getBaseName(const char* filePath);

#endif //_FILENAME_H_
