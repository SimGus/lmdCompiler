#ifndef _FILENAME_H_
#define _FILENAME_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"

char* getDirName(const char* filePath);
char* getBaseName(const char* filePath);

char* getOutputNameFromInputName(const char* inputFileName);

char* addTexExtension(const char* fileName);
char* addPdfExtension(const char* fileName);

#endif //_FILENAME_H_
