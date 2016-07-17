#include "texToPdf.h"

STATUS compileTexToPdf(const char* texFileName, const char* pdfFileName)
{
   char* noExtensionPdfFileName = getFileNameWithoutExtension(pdfFileName);
   if (noExtensionPdfFileName == NULL)
   {
      ERROR_MSG("compileTexToPdf", "Couldn't get the pdf file name without extension, using tex file name as pdf output.");
      noExtensionPdfFileName = getFileNameWithoutExtension(texFileName);
      if (noExtensionPdfFileName == NULL)
      {
         ERROR_MSG("compileTexToPdf", "Couldn't get the tex file name without extension for the pdf output. NO PDF OUTPUT WILL BE MADE.");
         return RETURN_FAILURE;
      }
   }

   char* command = malloc( (strlen(PDFLATEX_COMMAND)+strlen(texFileName)+strlen(noExtensionPdfFileName)+2)*sizeof(char) );
   sprintf(command, "%s%s %s", PDFLATEX_COMMAND, noExtensionPdfFileName, texFileName);

   puts("\nCompiling tex file to pdf file...");
   printf("%s\n\n", command);

   system(command);

   free(command);
   free(noExtensionPdfFileName);

   puts("Done");

   return RETURN_SUCCESS;
}
