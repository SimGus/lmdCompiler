#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "error.h"

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

char* argToOutputFileName(const char* outputArg)
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
	}
	else
	{
		filename = malloc( (argLength+5)*sizeof(char) );
		strcpy(filename, outputArg);
		strcat(filename, ".tex");
	}
	return filename;
}

void replaceExtension(char* string)
{
	int length = strlen(string);
	if (length <= 3)
	{
		char msg[256] = "couldn't replace .lmd extension with .tex : output file name is ";
		sprintf(msg, "%s%s", msg, string);
		WARNING_FUNC("replaceExtension", msg);
		return;
	}

	string[length-3] = 't';
	string[length-2] = 'e';
	string[length-1] = 'x';
}

void printHelp()
{
	puts("Markdown to LaTeX\tVersion : \u03B10.2");
	puts("Written by Simon Gustin in 2016");
	puts("\tUse : ./program [-o OutputFileName] InputFileName");
}

int main(int argc, char *argv[])
{
	bool inputSet = false, outputSet = false;
	char *inputFileName, *outputFileName;
	if (argc <= 1 || strcmp(argv[1], "--help") == 0)
	{
		printHelp();
		return EXIT_SUCCESS;
	}
	else
	{
		for (int i=1; i<argc; i++)
		{
			if (strcmp(argv[i], "-o") == 0)
			{
				if (outputSet)
				{
					ERROR_MSG("main", "invalid arguments (try --help)");
					free(outputFileName);
					if (inputSet)
						free(inputFileName);
					return EXIT_FAILURE;
				}

				i++;
				outputFileName = argToOutputFileName(argv[i]);
				outputSet = true;
				//printf("Set out to %s\n", outputFileName);
			}
			else
			{
				if (inputSet)
				{
					ERROR_MSG("main", "invalid arguments (try --help)");
					free(inputFileName);
					if (outputSet)
						free(outputFileName);
					return EXIT_FAILURE;
				}

				inputFileName = argToInputFileName(argv[i]);
				inputSet = true;
				//printf("Set in to %s\n", inputFileName);
			}
		}

		if (!outputSet)
		{
			outputFileName = strdup(inputFileName);
			replaceExtension(outputFileName);
			if (strcmp(outputFileName, inputFileName) == 0)
			{
				free(inputFileName);
				free(outputFileName);
				ERROR_MSG("main", "output file name is the same as input file name");
				return EXIT_FAILURE;
			}
		}
	}

	printf("input : %s\n", inputFileName);
	printf("output : %s\n", outputFileName);

	free(inputFileName);
	free(outputFileName);

	puts("Done");

	return EXIT_SUCCESS;
}
