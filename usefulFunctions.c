#include "usefulFunctions.h"

void reallocate(char** string, unsigned int newSize)
{
	char* tmpString = malloc(newSize*sizeof(char));
	strncpy(tmpString, *string, newSize);
	free(*string);
	*string = tmpString;
}
