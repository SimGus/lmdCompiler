#include "filename.h"

char* getDirName(const char* filePath)
{
	char* lastSlash = strrchr(filePath, '/');
	if (lastSlash == NULL)
		return NULL;

	char* tmp = malloc( (strlen(filePath)+1)*sizeof(char) );

	unsigned int i;
	for (i=0; &filePath[i]!=lastSlash+1 && filePath[i]!='\0'; i++)
		tmp[i] = filePath[i];
	tmp[i] = '\0';

	char* dirName = malloc( (strlen(tmp)+1)*sizeof(char) );
	strcpy(dirName, tmp);
	free(tmp);

	return dirName;
}

char* getBaseName(const char* filePath)
{
	char* lastSlash = strrchr(filePath, '/');
	if (lastSlash == NULL)
		return NULL;

	char* baseName = malloc( strlen(lastSlash)*sizeof(char) );
	strcpy(baseName, lastSlash+1);

	return baseName;
}

char* getOutputNameFromInputName(const char* inputFileName)
{
	if (inputFileName == NULL)
	{
		WARNING_FUNC("getOutputNameFromInputName", "Tried to get output file name from input file name while input file name wasn't set");
		return NULL;
	}
	char* outputFileName;

	unsigned short inputLength = strlen(inputFileName);
	if (inputLength <= 3)
	{
		outputFileName = malloc( (inputLength+1)*sizeof(char) );
		strcpy(outputFileName, inputFileName);
		return outputFileName;
	}

	if (inputFileName[inputLength-4] == '.')
	{
		outputFileName = malloc( (inputLength-3)*sizeof(char) );
		strncpy(outputFileName, inputFileName, inputLength-4);
		outputFileName[inputLength-4] = '\0';
		return outputFileName;
	}

	outputFileName = malloc( (inputLength+1)*sizeof(char) );
	strcpy(outputFileName, inputFileName);
	return outputFileName;
}

char* addTexExtension(const char* fileName)
{
	if (fileName == NULL || strcmp(fileName, "") == 0)
	{
		ERROR_MSG("addTexExtension", "Tried to add extension to empty file name");
		return NULL;
	}

	unsigned short fileNameLength = strlen(fileName);
	char* fileNameWithExtension = malloc( (fileNameLength+5)*sizeof(char) );
	strcpy(fileNameWithExtension, fileName);

	fileNameWithExtension[fileNameLength] = '.';
	fileNameWithExtension[fileNameLength+1] = 't';
	fileNameWithExtension[fileNameLength+2] = 'e';
	fileNameWithExtension[fileNameLength+3] = 'x';

	return fileNameWithExtension;
}

char* addPdfExtension(const char* fileName)
{
	if (fileName == NULL || strcmp(fileName, "") == 0)
	{
		ERROR_MSG("addPdfExtension", "Tried to add extension to empty file name");
		return NULL;
	}

	unsigned short fileNameLength = strlen(fileName);
	char* fileNameWithExtension = malloc( (fileNameLength+5)*sizeof(char) );
	strcpy(fileNameWithExtension, fileName);

	fileNameWithExtension[fileNameLength] = '.';
	fileNameWithExtension[fileNameLength+1] = 'p';
	fileNameWithExtension[fileNameLength+2] = 'd';
	fileNameWithExtension[fileNameLength+3] = 'f';

	return fileNameWithExtension;
}
