#include "texToPdf.h"

STATUS compileTexToPdf(const char* outputFileName)
{
   char* texFileName = addTexExtension(outputFileName);
   if (texFileName == NULL)
   {
      ERROR_MSG("compileTexToPdf", "Couldn't get the tex file name without extension for the pdf output. NO PDF OUTPUT WILL BE PRODUCED.");
      return RETURN_FAILURE;
   }

   char* command = malloc( (strlen(PDFLATEX_COMMAND)+strlen(texFileName)+1)*sizeof(char) );
   sprintf(command, "%s%s", PDFLATEX_COMMAND, texFileName);

   puts("\nCompiling tex file to pdf file...");
   printf("%s\n\n", command);

   system(command);

   free(command);
   free(texFileName);

   return RETURN_SUCCESS;
}
