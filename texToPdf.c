#include "texToPdf.h"

STATUS compileTexToPdf(const char* outputFileName, bool displayPdflatexStdout, bool keepTmpFiles)
{
   char* texFileName = addTexExtension(outputFileName);
   if (texFileName == NULL)
   {
      ERROR_MSG("compileTexToPdf", "Couldn't get the tex file name without extension for the pdf output. NO PDF OUTPUT WILL BE PRODUCED.");
      return RETURN_FAILURE;
   }

   //==================Make command=================================
   char* command;
   if (displayPdflatexStdout)
   {
      command = malloc( (strlen(PDFLATEX_COMMAND)+strlen(texFileName)+1)*sizeof(char) );
      sprintf(command, "%s%s", PDFLATEX_COMMAND, texFileName);
   }
   else
   {
      command = malloc( (strlen(PDFLATEX_COMMAND_REDIRECT)+strlen(texFileName)-1)*sizeof(char) );
      sprintf(command, PDFLATEX_COMMAND_REDIRECT, texFileName);
   }

   puts("\nCompiling tex file to pdf file...");
   printf("%s\n\n", command);

   system(command);

   free(command);
   free(texFileName);

   if (!keepTmpFiles)
   {
      //=====================Delete temporary files==========================
      char* fileNameToDelete = malloc( (strlen(outputFileName)+5)*sizeof(char) );

      sprintf(fileNameToDelete, "%s%s", outputFileName, ".aux");
      if (deleteFile(fileNameToDelete) != RETURN_SUCCESS)
      {
         WARNING_FUNC("compileTexToPdf", "There seemed to have a problem deleting temporary .aux file");
      }

      sprintf(fileNameToDelete, "%s%s", outputFileName, ".out");
      if (deleteFile(fileNameToDelete) != RETURN_SUCCESS)
      {
         WARNING_FUNC("compileTexToPdf", "There seemed to have a problem deleting temporary .out file");
      }

      sprintf(fileNameToDelete, "%s%s", outputFileName, ".log");
      if (deleteFile(fileNameToDelete) != RETURN_SUCCESS)
      {
         WARNING_FUNC("compileTexToPdf", "There seemed to have a problem deleting temporary .log file");
      }

      sprintf(fileNameToDelete, "%s%s", outputFileName, ".nav");
      if (deleteFile(fileNameToDelete) != RETURN_SUCCESS)
      {
         WARNING_FUNC("compileTexToPdf", "There seemed to have a problem deleting temporary .nav file");
      }

      sprintf(fileNameToDelete, "%s%s", outputFileName, ".snm");
      if (deleteFile(fileNameToDelete) != RETURN_SUCCESS)
      {
         WARNING_FUNC("compileTexToPdf", "There seemed to have a problem deleting temporary .snm file");
      }

      sprintf(fileNameToDelete, "%s%s", outputFileName, ".toc");
      if (deleteFile(fileNameToDelete) != RETURN_SUCCESS)
      {
         WARNING_FUNC("compileTexToPdf", "There seemed to have a problem deleting temporary .toc file");
      }

      free(fileNameToDelete);
   }

   return RETURN_SUCCESS;
}
