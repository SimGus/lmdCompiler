#include "compiler.h"

FILE* inputFile = NULL;

STATUS compile(const char* inputFileName, const char* outputFileName)
{
   FILE *outputFile = NULL, *tmpBodyOutputFile = NULL;

   //open input file
   inputFile = fopen(inputFileName, "r");
   if (inputFile == NULL)
   {
      char msg[256];
      sprintf(msg, "%s %s %s", "Unable to open file", inputFileName, "(doesn't exist?)");
      ERROR_MSG("compile", msg);
      return RETURN_FAILURE;
   }

   //open temporary file
   char* tmpBodyOutputFilePath = getTmpFileName(outputFileName);
   tmpBodyOutputFile = fopen(tmpBodyOutputFilePath, "w+");
   if (tmpBodyOutputFile == NULL)
   {
      ERROR_MSG("compile", "Couldn't create temporary file");
      fclose(inputFile);
      free(tmpBodyOutputFilePath);
      return RETURN_FAILURE;
   }

   //translate line by line
   initPreamble();

   char* line;
   while ((line = getNextLineFromFile()) != NULL)
   {

      free(line);
   }
   fclose(inputFile);

   //open output file
   outputFile = fopen(outputFileName, "w");
   if (outputFile == NULL)
   {
      ERROR_MSG("compile", "Couldn't open or create output file");
      fclose(tmpBodyOutputFile);
      deleteFile(tmpBodyOutputFilePath);
      free(tmpBodyOutputFilePath);
      return RETURN_FAILURE;
   }

   //write preamble to output file
   if (insertPreamble(tmpBodyOutputFile, outputFile) != RETURN_SUCCESS)
   {
      fclose(outputFile);
      fclose(tmpBodyOutputFile);
      deleteFile(tmpBodyOutputFilePath);
      free(tmpBodyOutputFilePath);
      return RETURN_FAILURE;
   }

   //close output file
   fclose(outputFile);

   //close temporary file
   fclose(tmpBodyOutputFile);
   if (deleteFile(tmpBodyOutputFilePath) != RETURN_SUCCESS)
   {
      free(tmpBodyOutputFilePath);
      return RETURN_FAILURE;
   }
   free(tmpBodyOutputFilePath);

   return RETURN_SUCCESS;
}

char* getTmpFileName(const char* outputFileName)
{
   char* tmpBodyOutputFilePath;
	char* tmpFileName = malloc( (strlen(outputFileName)+1)*sizeof(char) );
	strcpy(tmpFileName, outputFileName);
	if (strcmp(dirname(tmpFileName), ".") == 0)
	{
		tmpBodyOutputFilePath = malloc( (strlen(TMP_OUTPUT_FILENAME)+1)*sizeof(char) );
		strcpy(tmpBodyOutputFilePath, TMP_OUTPUT_FILENAME);
	}
   else
	{
		strcpy(tmpFileName, outputFileName);
		tmpBodyOutputFilePath = malloc( (strlen(outputFileName)+strlen(TMP_OUTPUT_FILENAME))*sizeof(char) );
		sprintf(tmpBodyOutputFilePath, "%s/%s", dirname(tmpFileName), TMP_OUTPUT_FILENAME);
	}
	free(tmpFileName);
   return tmpBodyOutputFilePath;
}

char* getNextLineFromFile()
{
   //count number of char until next \n
   char current = '\0';
   int count;
   for (count=0; current!='\n' && current!=EOF; count++)
      current = fgetc(inputFile);

   if (current == EOF)
      return NULL;

   //put cursor back at the beginning of the line
   fseek(inputFile, -count, SEEK_CUR);

   //get line
   char* line = malloc( (count+2)*sizeof(char) );
   fgets(line, count, inputFile);

   //get the cursor to the next line
   fseek(inputFile, 1, SEEK_CUR);

   return line;
}

STATUS deleteFile(const char* filePath)
{
   if (unlink(filePath) != 0)
   {
      char msg[256] = "Couldn't delete temporary file";
      snprintf(msg, 256-strlen(msg), "%s %s", msg, filePath);
      ERROR_MSG("deleteFile", msg);
      return RETURN_FAILURE;
   }
   return RETURN_SUCCESS;
}
