#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "compiler.h"
#include "filename.h"
#include "texToPdf.h"

void printHelp()
{
	puts("Markdown to LaTeX\tVersion : \u03B10.3");
	puts("Written by Simon Gustin in 2016");
	puts("\tUse : ./PROGRAM [OPTIONS] InputFileName");
	puts("\t\tOptions :");
	puts("\t\t\t-o, --output-name :");
	puts("\t\t\t\tspecify the name of the output file.");
	puts("\t\t\t-t, --file-type :");
	puts("\t\t\t\tspecify the type of the output file.");
	puts("\t\t\t\tThe different possible types are 'tex' and 'pdf'.");
	puts("\t\t\t\tBoth can be output by using the conjunction '&'.");
}

void freeFileNames(char* inputFileName, char* outputTexFileName, char* outputPdfFileName)
{
	if (inputFileName != NULL)
		free(inputFileName);
	if (outputTexFileName != NULL)
		free(outputTexFileName);
	if (outputPdfFileName != NULL)
		free(outputPdfFileName);
}

int main(int argc, char *argv[])
{
	char *inputFileName = NULL, *outputTexFileName = NULL, *outputPdfFileName = NULL;
	OutputType outputFilesType = BOTH;//BOTH by default
	if (argc <= 1 || strcmp(argv[1], "--help") == 0)
	{
		printHelp();
		return EXIT_SUCCESS;
	}
	else
	{
		for (int i=1; i<argc; i++)
		{
			if (strcmp(argv[i], "-o")==0 || strcmp(argv[i], "--output-name")==0)
			{
				if (outputTexFileName != NULL || outputPdfFileName != NULL)
				{
					ERROR_MSG("main", "Invalid arguments (try --help)");
					freeFileNames(inputFileName, outputTexFileName, outputPdfFileName);
					return EXIT_FAILURE;
				}

				i++;
				outputTexFileName = argToOutputFileName(argv[i], TEX);

				i++;
				outputPdfFileName = argToOutputFileName(argv[i], PDF);
			}
			else if (strcmp(argv[i], "-t")==0 || strcmp(argv[i], "--file-type")==0)
			{
				i++;
				if (strcmp(argv[i], "tex") == 0)
					outputFilesType = TEX;
				else if (strcmp(argv[i], "pdf") == 0)
					outputFilesType = PDF;
				else if (strcmp(argv[i], "tex&pdf")==0 || strcmp(argv[i], "pdf&tex")==0)
					outputFilesType = BOTH;
				else
				{
					WARNING_FUNC("main", "Invalid argument for output files type. Setting file type to tex&pdf.");
				}
			}
			else
			{
				if (inputFileName != NULL)
				{
					ERROR_MSG("main", "Invalid arguments (try --help)");
					freeFileNames(inputFileName, outputTexFileName, outputPdfFileName);
					return EXIT_FAILURE;
				}

				inputFileName = argToInputFileName(argv[i]);
			}
		}
	}

	if (outputTexFileName == NULL)
	{
		outputTexFileName = malloc( (strlen(inputFileName)+1)*sizeof(char) );
		strcpy(outputTexFileName, inputFileName);
		replaceExtension(outputTexFileName, TEX);
		if (strcmp(outputTexFileName, inputFileName) == 0)
		{
			freeFileNames(inputFileName, outputTexFileName, outputPdfFileName);
			ERROR_MSG("main", "TEX output file name is the same as input file name");
			return EXIT_FAILURE;
		}
	}
	if (outputPdfFileName == NULL)
	{
		outputPdfFileName = malloc( (strlen(inputFileName)+1)*sizeof(char) );
		strcpy(outputPdfFileName, inputFileName);
		replaceExtension(outputPdfFileName, PDF);
		if (strcmp(outputPdfFileName, inputFileName) == 0)
		{
			freeFileNames(inputFileName, outputTexFileName, outputPdfFileName);
			ERROR_MSG("main", "PDF output file name is the same as input file name");
			return EXIT_FAILURE;
		}
	}

	printf("input : '%s'\n", inputFileName);
	printf("tex output : '%s'\n", outputTexFileName);
	printf("pdf output : '%s'\n", outputPdfFileName);

	STATUS err = compile(inputFileName, outputTexFileName);
	if (err != RETURN_SUCCESS)
	{
		puts("The compilation ended with an error code");
		freeFileNames(inputFileName, outputTexFileName, outputPdfFileName);
		return EXIT_FAILURE;
	}
	else
	{
		puts("Done");

		if (outputFilesType == PDF || outputFilesType == BOTH)
		{
			compileTexToPdf(outputTexFileName, outputPdfFileName);
		}
	}

	freeFileNames(inputFileName, outputTexFileName, outputPdfFileName);

	return EXIT_SUCCESS;
}
