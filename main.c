#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "compiler.h"

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
		char msg[256] = "Couldn't replace .lmd extension with .tex : output file name is";
		snprintf(msg, 256-strlen(msg), "%s %s", msg, string);
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
	char *inputFileName = NULL, *outputFileName = NULL;
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
				if (outputFileName != NULL)
				{
					ERROR_MSG("main", "Invalid arguments (try --help)");
					free(outputFileName);
					if (inputFileName != NULL)
						free(inputFileName);
					return EXIT_FAILURE;
				}

				i++;
				outputFileName = argToOutputFileName(argv[i]);
			}
			else
			{
				if (inputFileName != NULL)
				{
					ERROR_MSG("main", "Invalid arguments (try --help)");
					free(inputFileName);
					if (outputFileName != NULL)
						free(outputFileName);
					return EXIT_FAILURE;
				}

				inputFileName = argToInputFileName(argv[i]);
			}
		}

		if (outputFileName == NULL)
		{
			outputFileName = malloc( (strlen(inputFileName)+1)*sizeof(char) );
			strcpy(outputFileName, inputFileName);
			replaceExtension(outputFileName);
			if (strcmp(outputFileName, inputFileName) == 0)
			{
				free(inputFileName);
				free(outputFileName);
				ERROR_MSG("main", "Output file name is the same as input file name");
				return EXIT_FAILURE;
			}
		}
	}

	printf("input : %s\n", inputFileName);
	printf("output : %s\n", outputFileName);

	STATUS err = compile(inputFileName, outputFileName);
	if (err != RETURN_SUCCESS)
		puts("The compilation ended with an error code");
	else
		puts("Done");

	free(inputFileName);
	free(outputFileName);

	return EXIT_SUCCESS;
}
