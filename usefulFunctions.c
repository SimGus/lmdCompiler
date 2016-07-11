#include "usefulFunctions.h"

void reallocate(char** string, unsigned int newSize)
{
	char* tmpString = malloc(newSize*sizeof(char));
	strncpy(tmpString, *string, newSize);
	free(*string);
	*string = tmpString;
}

char* duplicateString(const char* source)
{
	char* destination = malloc( (strlen(source)+1)*sizeof(char) );
	strcpy(destination, source);
	return destination;
}
