#include "filename.h"

char* argToInputFileName(const char* inputArg)
{
	char* filename;
	int argLength = strlen(inputArg);

	if (argLength > 3
		&& inputArg[argLength-4] == '.'
		&& inputArg[argLength-3] == 'l'
		&& inputArg[argLength-2] == 'm'
		&& inputArg[argLength-1] == 'd')//inputArg ends in ".lmd"
	{
		filename = malloc( (argLength+1)*sizeof(char) );
		strcpy(filename, inputArg);
	}
	else
	{
		filename = malloc( (argLength+5)*sizeof(char) );
		strcpy(filename, inputArg);
		strcat(filename, ".lmd");
	}
	return filename;
}

char* argToOutputFileName(const char* outputArg, OutputType fileType)
{
	char* filename;
	int argLength = strlen(outputArg);

	if (argLength > 3
		&& outputArg[argLength-4] == '.'
		&& outputArg[argLength-3] == 't'
		&& outputArg[argLength-2] == 'e'
		&& outputArg[argLength-1] == 'x')//outputArg ends in ".tex"
	{
		filename = malloc( (argLength+1)*sizeof(char) );
	   strcpy(filename, outputArg);
      if (fileType == PDF)
      {
         filename[argLength-3] = 'p';
         filename[argLength-2] = 'd';
         filename[argLength-1] = 'f';
      }
	}
	else
	{
		filename = malloc( (argLength+5)*sizeof(char) );
		strcpy(filename, outputArg);
      if (fileType == TEX)
		    strcat(filename, ".tex");
      else if (fileType == PDF)
         strcat(filename, ".pdf");
      else
      {
         ERROR_MSG("argToOutputFileName", "Tried to set file name with an output extension. Output file may not have the right extension.");
      }
	}
	return filename;
}

void replaceExtension(char* string, OutputType fileType)
{
	int length = strlen(string);
	if (length <= 3)
	{
		char msg[256] = "Couldn't replace .lmd extension with .tex : output file name is";
		snprintf(msg, 256, "%s %s", msg, string);
		WARNING_FUNC("replaceExtension", msg);
		return;
	}

   if (fileType == TEX)
   {
   	string[length-3] = 't';
   	string[length-2] = 'e';
   	string[length-1] = 'x';
   }
   else if (fileType == PDF)
   {
      string[length-3] = 'p';
   	string[length-2] = 'd';
   	string[length-1] = 'f';
   }
   else
   {
      ERROR_MSG("replaceExtension", "Tried to replace the extension with an unknown extension. The output file will not have the right extensions.");
   }
}

char* getFileNameWithoutExtension(const char* fileName)
{
	unsigned short fileNameLength = strlen(fileName);
	if (fileName[fileNameLength-5] == '.')
		return NULL;

	char* noExtensionFileName = malloc( (fileNameLength-3)*sizeof(char) );
	strcpy(noExtensionFileName, fileName);
	noExtensionFileName[fileNameLength-4] = '\0';

	return noExtensionFileName;
}

char* getDirName(const char* filePath)
{
	char* lastSlash = strrchr(filePath, '/');
	if (lastSlash == NULL)
		return NULL;

	char* tmp = malloc( (strlen(filePath)+1)*sizeof(char) );

	unsigned int i;
	for (i=0; &filePath[i]!=lastSlash && filePath[i]!='\0'; i++)
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
	strcpy(baseName, lastSlash);

	return baseName;
}
