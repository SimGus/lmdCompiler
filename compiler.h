#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
#include <unistd.h>

#include "error.h"
#include "preamble.h"

#define TMP_OUTPUT_FILENAME "tmpBody.tmp"

STATUS compile(const char* inputFileName, const char* outputFileName);
void interpretLine(FILE* outputFile, const char* line);

char* getNextLineFromFile();

char* getTmpFileName(const char* outputFileName);
STATUS deleteFile(const char* filePath);

#endif //_COMPILER_H_
