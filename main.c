#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "error.h"
#include "compiler.h"
#include "files.h"
#include "texToPdf.h"
#include "usefulFunctions.h"

#define MAX_PATH_LENGTH	1024

typedef enum
{
   TEX, PDF, BOTH
} OutputType;

void printHelp()
{
	puts("Markdown to LaTeX\tVersion : \u03B21.0");
	puts("Written by Simon Gustin in 2016");
	puts("\tUse : ./PROGRAM [OPTIONS] InputFileName");
	puts("\t\tOptions :");
	puts("\t\t\t-o, --output-name :");
	puts("\t\t\t\tspecify the name of the output file.");
	puts("\t\t\t-t, --output-type :");
	puts("\t\t\t\tspecify the type of the output file.");
	puts("\t\t\t\tThe different possible types are 'tex' and 'pdf'.");
	puts("\t\t\t\tBoth can be output by using the conjunction '-'.");
	puts("\t\t\t-K, --keep-tmp-files :");
	puts("\t\t\t\tDon't delete temporary files, which are the temporary file that LMD produces and several files produced by pdflatex.");
	puts("\t\t\t-k, --keep-pdflatex-tmp-files :");
	puts("\t\t\t\tDon't delete the temporary files produced by pdflatex.");
	puts("\t\t\t--keep-lmd-tmp-files :");
	puts("\t\t\t\tDon't delete the temporary file produced by LMD.");
	puts("\t\t\t--print-pdflatex-stdout :");
	puts("\t\t\t\tPrint the standard output of the pdflatex command.");
	puts("\tExample : ./lmd -o outputFile -K --output-type=tex-pdf file/input/inputFile.lmd");
}

void freeFileNames(char* workingDirectory, char* inputFileName, char* outputFileName)
{
	if (workingDirectory != NULL)
		free(workingDirectory);
	if (inputFileName != NULL)
		free(inputFileName);
	if (outputFileName != NULL)
		free(outputFileName);
}

int main(int argc, char* argv[])
{
	char *workingDirectory = NULL, *inputFileName = NULL, *outputFileName = NULL;
	OutputType outputFilesType = BOTH;
	bool keepPdflatexFiles = false, keepTmpLmdFiles = false, displayPdflatexStdout = false;

	if (argc <= 1 || strcmp(argv[1], "--help") == 0)
	{
		printHelp();
		return EXIT_SUCCESS;
	}

	//=============Interpret arguments=================
	for (unsigned char i=1; i<argc; i++)//interpret all arguments
	{
		if (strcmp(argv[i], "-o") == 0 || strstr(argv[i], "--output-name=") != NULL)
		{
			if (outputFileName != NULL)
			{
				ERROR_MSG("main", "Invalid arguments (try --help)");
				freeFileNames(workingDirectory, inputFileName, outputFileName);
				return EXIT_FAILURE;
			}

			if (strcmp(argv[i], "-o") == 0)
			{
				if (i+1 == argc)
				{
					ERROR_MSG("main", "Invalid arguments (try --help)");
					freeFileNames(workingDirectory, inputFileName, outputFileName);
					return EXIT_FAILURE;
				}

				outputFileName = duplicateString(argv[i+1]);
				i++;
			}
			else//argv[i] contains "--output-name="
			{
				char* pointerToEqual = strchr(argv[i], '=');
				outputFileName = duplicateString(pointerToEqual+1);
			}
		}
		else if (strcmp(argv[i], "-t") == 0 || strstr(argv[i], "--output-type=") != NULL)
		{
			if (strcmp(argv[i], "-t") == 0)
			{
				if (i+1 == argc)
				{
					ERROR_MSG("main", "Invalid arguments (try --help)");
					freeFileNames(workingDirectory, inputFileName, outputFileName);
					return EXIT_FAILURE;
				}

				i++;
				if (strcmp(argv[i], "tex") == 0)
					outputFilesType = TEX;
				else if (strcmp(argv[i], "pdf") == 0)
					outputFilesType = PDF;
				else if (strcmp(argv[i], "tex-pdf") == 0 || strcmp(argv[i], "pdf-tex") == 0)
					outputFilesType = BOTH;
				else
				{
					ERROR_MSG("main", "Invalid arguments for output type (tex, pdf or tex-pdf)");
					freeFileNames(workingDirectory, inputFileName, outputFileName);
					return EXIT_FAILURE;
				}
			}
			else//argv[i] contains "--output-type="
			{
				if (strcmp(argv[i], "--output-type=tex") == 0)
					outputFilesType = TEX;
				else if (strcmp(argv[i], "--output-type=pdf") == 0)
					outputFilesType = PDF;
				else if (strcmp(argv[i], "--output-type=tex-pdf") == 0 || strcmp(argv[i], "--output-type=pdf-tex") == 0)
					outputFilesType = BOTH;
				else
				{
					ERROR_MSG("main", "Invalid argument for output type (tex, pdf or tex-pdf)");
					freeFileNames(workingDirectory, inputFileName, outputFileName);
					return EXIT_FAILURE;
				}
			}
		}
		else if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--keep-pdflatex-tmp-files") == 0)
		{
			keepPdflatexFiles = true;
		}
		else if (strcmp(argv[i], "-K") == 0 || strcmp(argv[i], "--keep-tmp-files") == 0)
		{
			keepPdflatexFiles = true;
			keepTmpLmdFiles = true;
		}
		else if (strcmp(argv[i], "--keep-lmd-tmp-files") == 0)
		{
			keepTmpLmdFiles = true;
		}
		else if (strcmp(argv[i], "--print-pdflatex-stdout") == 0)
		{
			displayPdflatexStdout = true;
		}
		else
		{
			if (inputFileName != NULL)
			{
				ERROR_MSG("main", "Invalid arguments (try --help)");
				freeFileNames(workingDirectory, inputFileName, outputFileName);
				return EXIT_FAILURE;
			}

			workingDirectory = getDirName(argv[i]);
			if (workingDirectory == NULL)//already in the working directory
			{
				workingDirectory = malloc(MAX_PATH_LENGTH*sizeof(char));
				getcwd(workingDirectory, MAX_PATH_LENGTH);
				if (workingDirectory == NULL)
				{
					ERROR_MSG("main", "Couldn't get working directory path.");
					freeFileNames(workingDirectory, inputFileName, outputFileName);
					return EXIT_FAILURE;
				}
			}

			char* tmp = getBaseName(argv[i]);
         if (tmp == NULL)
				tmp = duplicateString(argv[i]);
         inputFileName = addLmdExtension(tmp);
         if (inputFileName != tmp)
            free(tmp);
		}
	}
	if (outputFileName == NULL)
		outputFileName = getOutputNameFromInputName(inputFileName);

	if (inputFileName == NULL || outputFileName == NULL)
	{
		ERROR_MSG("main", "Something went wrong when setting file names");
		freeFileNames(workingDirectory, inputFileName, outputFileName);
		return EXIT_FAILURE;
	}
	else if (strcmp(inputFileName, "") == 0 || strcmp(outputFileName, "") == 0)
	{
		ERROR_MSG("main", "The input or output file name specified seemed to be either empty or a directory");
		freeFileNames(workingDirectory, inputFileName, outputFileName);
		return EXIT_FAILURE;
	}

	//=================Display arguments=========================
	printf("Working directory : %s\n", workingDirectory);
	printf("Input : %s\n", inputFileName);
	printf("Output : %s\n", outputFileName);
	switch (outputFilesType)
	{
		case TEX:
			puts("Output type : tex");
			break;
		case PDF:
			puts("Output type : pdf");
			break;
		case BOTH:
			puts("Output type : tex and pdf");
			break;
	}

	//===================Changing current working directory=========================
	if (workingDirectory != NULL)
	{
		if (chdir(workingDirectory) != 0)
		{
			ERROR_MSG("main", "Unable to change current working directory");
			freeFileNames(workingDirectory, inputFileName, outputFileName);
			return EXIT_FAILURE;
		}
	}

	//===================Translate to tex file==================================
	char* texFileName = addTexExtension(outputFileName);
	STATUS err = compile(inputFileName, texFileName, keepTmpLmdFiles);
	if (err != RETURN_SUCCESS)
	{
		puts("The compilation to tex file ended with an error code.");
		freeFileNames(workingDirectory, inputFileName, outputFileName);
		return EXIT_FAILURE;
	}
	puts("\nTEX translation over.");

	//=====================Translate to pdf file and delete temporary files if needed==============================
	if (outputFilesType == PDF || outputFilesType == BOTH)
	{
		err = compileTexToPdf(outputFileName, displayPdflatexStdout, keepPdflatexFiles);
		if (err != RETURN_SUCCESS)
		{
			puts("The compilation to pdf file ended with an error code.");
			freeFileNames(workingDirectory, inputFileName, outputFileName);
			return EXIT_FAILURE;
		}
		puts("PDF translation over.");
	}

   if (outputFilesType == PDF)
   {
      //=====================Delete tex file===========================================
      puts("Deleting tex file...");
      if (deleteFile(texFileName) != RETURN_SUCCESS)
      {
         WARNING_FUNC("main", "tex file wasn't deleted");
      }
   }
   free(texFileName);

	puts("Done.");

	freeFileNames(workingDirectory, inputFileName, outputFileName);
	return EXIT_SUCCESS;
}
