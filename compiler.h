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
STATUS interpretLine(FILE* bodyOutputFile, const char* line);

/*
 * @return :   a string containing the next unread line of the input file
 *             NULL if the input file as been entirely read
 * @post : the string return MUST be freed
 */
char* getNextLineFromFile();

/*
 * @return : number of spaces at the beginning of the line
 */
unsigned short getIndentation(const char* line);

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
 * Sets global variable workingDirName to the path of the working directory
 */
void setWorkingDirectoryName(const char* outputFileName);

/*
 * @return :   true if the file with path imgFileName exists
 *             false otherwise
 */
bool imageFileExists(const char* imgFileName);

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
 * @return :   true if the useful part of line is a '['
 *             false if it isn't
 */
bool isMultilinePlainTextOpeningTag(const char* line);

/*
 * same than the function described right above but for closgin tag ']'
 */
bool isMultilinePlainTextClosingTag(const char* line);

/*
 * @return :   true if the line is in the form "<img image.png [label] [width:height]>"
 *             false otherwise
 */
bool isImageLine(const char* line);

/*
 * writes nbAlinea tabulations to file
 */
void writeAlinea(FILE* file);

/*
 * @pre : line must be a valid line characterizing an image file
 * @return :   the name (or path) of the image
 *             NULL if there was a problem
 * @post : the returned value MUST be freed
 */
char* pickImageFileName(const char* line);

/*
 * @pre : line must be a valid line characterizing an image inclusion
 * @return :   a string containing the caption of the image
 *             NULL if no label was found
 * @post : the returned value MUST be freed
 */
char* pickImageLabel(const char* line);

/*
 * @pre : firstURLIndex must be the index in line pointing to the 'h' of "http"
 * @return :   a string containing the url
 *             NULL if there was a problem
 * @post : the string returned MUST be freed
 */
char* pickURL(const char* line, unsigned int firstURLIndex);

/*
 * @pre : firstURLIndex must be the index in line pointing to the 'h' of "http"
 * @return :   a string containing the label that will link to the url
 *             NULL if no label was found
 * @post : the string (not NULL) returned MUST be freed
 */
char* pickURLLabel(const char* line, unsigned int firstURLIndex);

/*
 * @return :   true if the line begins in "+ " or "+\t"
 *             false otherwise
 */
bool isItemizeLine(const char* line);

/*
 * @return :   a string containing an item from a list
 *             NULL if there was a problem
 * @post : the string returned MUST be freed
 */
char* pickItemFromItemize(const char* line);

/*
 * Detects and writes an itemize to bodyOutputFile
 * @pre : line must be an itemize line
 */
void writeItemize(FILE* bodyOutputFile, const char* line);

/*
 * Writes line as an itemize item in bodyOutputFile
 * @pre : line MUST be an itemize line (see above)
 */
void writeItemizeItem(FILE* bodyOutputFile, const char* line);

/*
 * @return :   true if the line begins in form "l. " or "l.\t"
 *             false otherwise
 */
bool isEnumLine(const char* line);

/*
 * Detects and writes an enumerate section to bodyOutputFile
 * @pre : line MUST be an enumeration line, hence begin in "l. " or "l.\t" (see above)
 */
void writeEnumerate(FILE* bodyOutputFile, const char* line);

/*
 * Writes line as an enumerate item in bodyOutputFile
 * @pre : line MUST be an enumerate line (see above)
 */
void writeEnumerateItem(FILE* bodyOutputFile, const char* line);

/*
 * @return :   a string containing an item from an enumerate section
 *             NULL if there was a problem
 * @post : the string returned MUST be freed
 */
char* pickItemFromEnumerate(const char* line);

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
