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
#include "usefulFunctions.h"
#include "pile.h"

#define TMP_OUTPUT_FILENAME "tmpBody.tmp"

#define MD_WARNING(lineNb, msg)  if (lineNb <= 0) \
                                    printf("WARNING (no line specified) :\n\t%s\n", msg); \
                                 else \
                                    printf("WARNING (line %d) :\n\t%s\n", lineNb, msg);
#define MD_ERROR(lineNb, msg)  if (lineNb <= 0) \
                                    printf("ERROR (no line specified) :\n\t%s\n", msg); \
                                 else \
                                    printf("ERROR (line %d) :\n\t%s\n", lineNb, msg);

/*
 * Opens and closes files and create temporary file.
 * Deletes temporary file.
 * Calls translateLineByLine and insertPreamble
 * @return :   RETURN_SUCCESS if everything worked fine
 *             RETURN_FAILURE if there was a problem with files or translation
 */
STATUS compile(const char* inputFileName, const char* outputFileName);
/*
 * Translates each line of the input file to bodyOutputFile
 */
void translateLineByLine(FILE* bodyOutputFile);
/*
 * Translate the line to bodyOutputFile
 * If there's need to interpret several lines at once, it calls getNextLineFromFile
 */
void interpretLine(FILE* bodyOutputFile, const char* line);

/*
 * @return :   a string containing the next unread line of the input file
 *             NULL if the input file as been entirely read
 * @post : the string return MUST be freed
 */
char* getNextLineFromFile();

/*
 * @return :   a string containing the name of the temporary file and its path if the program was run from another directory
 * @post : the string returned MUSt be freed
 */
char* getTmpFileName(const char* outputFileName);
/*
 * Deletes the file with path filePath
 * @return :   RETURN_SUCCESS if everything worked fine
 *             RETURN_FAILURE if the file couldn't be deleted
 */
STATUS deleteFile(const char* filePath);

/*
 * @return :   a string containing the useful part of a line containing the name of a new section
 *                (thus comments, spaces and hashtags are remmoved)
 *             NULL if the line is not a title line
 * @post : the string returned MUST be freed
 */
char* getTitleOfPart(const char* line);

/*
 * @return :   the index of the part of line where the comment begins (the '%')
 *             -1 if no comment was found
 * @post : DO NOT FREE the string returned
 */
int getFirstIndexOfComment(const char* line);

/*
 * Removes useless spaces in string (the string changes), in general there are spaces there because there is a comment afterwards in the input
 */
void removeUselessSpaces(char* string);

/*
 * Translates source (in markdown) to destination (in LaTeX)
 * @post : *destination MUST be freed
 */
void translateString(const char* source, char** destination);

/*
 * Translates string (in mardown) in LaTeX and writes it directly in the file bodyOutputFile
 */
void translateToFile(FILE* bodyOutputFile, const char* string);

#endif //_COMPILER_H_
