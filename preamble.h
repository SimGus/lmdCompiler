#ifndef _PREAMBLE_H_
#define _PREAMBLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>

#include "error.h"
#include "usefulFunctions.h"

#define FGETS_LINE_LENGTH  1024

typedef struct
{
   bool containsStrikethroughs;
   bool containsEnumerations;
   bool containsImages;
   bool containsLinks;
   char* title;
   char* titleComments;
} Preamble;

void initPreamble();

void addStrikethroughsToPreamble();
void addEnumToPreamble();
void addImagesToPreamble();
void addLinksToPreamble();

void addLineToTitle(const char* newLine);
void addCommentToTitle(const char* newComment);

int insertPreamble(FILE* tmpBodyOutputFile, FILE* outputFile);

void freePreamble();

#endif //_PREAMBLE_H_
