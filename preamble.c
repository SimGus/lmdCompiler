#include "preamble.h"

static Preamble preamble;

void initPreamble()
{
   preamble.containsStrikethroughs = false;
   preamble.containsEnumerations = false;
   preamble.containsImages = false;
   preamble.containsLinks = false;
}

void addStrikethroughsToPreamble()
{
   preamble.containsStrikethroughs = true;
}

void addEnumToPreamble()
{
   preamble.containsEnumerations = true;
}

void addImagesToPreamble()
{
   preamble.containsImages = true;
}

void addLinksToPreamble()
{
   preamble.containsLinks = true;
}

void addLineToTitle(const char* newLine)
{
   if (newLine != NULL)
   {
      if (preamble.title == NULL)
      {
         preamble.title = malloc( (strlen(newLine)+1)*sizeof(char) );
         strcpy(preamble.title, newLine);
      }
      else
      {
         reallocate(&(preamble.title), strlen(preamble.title)+strlen(newLine)+3);
         sprintf(preamble.title, "%s\\\\%s", preamble.title, newLine);
      }
   }
   else
   {
      WARNING_FUNC("addLineToTitle", "Tried to add an empty line to title");
   }
}

void addCommentToTitle(const char* newComment)
{
   if (newComment != NULL || strcmp(newComment, "%") != 0 || strcmp(newComment, "% ") != 0)
   {
      if (preamble.titleComments == NULL)
      {
         preamble.titleComments = malloc( (strlen(newComment)+1)*sizeof(char) );
         strcpy(preamble.titleComments, newComment);
      }
      else
      {
         reallocate(&(preamble.titleComments), strlen(preamble.titleComments)+strlen(newComment)+2);
         sprintf(preamble.titleComments, "%s\n%s", preamble.titleComments, newComment);
      }
   }
   else
   {
      WARNING_FUNC("addCommentToTitle", "Tried to add an empty comment to the title");
   }
}

int insertPreamble(FILE* tmpBodyOutputFile, FILE* outputFile)
{
   fputs("\\documentclass{article}\n\n", outputFile);

   fputs("\\usepackage[francais]{babel}\n", outputFile);
   fputs("\\usepackage[utf8]{inputenc}\n", outputFile);
   fputs("\\usepackage[T1]{fontenc}\n\n", outputFile);

   if (preamble.containsStrikethroughs)
      fputs("\\usepackage[normalem]{ulem}\n", outputFile);
   if (preamble.containsEnumerations)
      fputs("\\usepackage{enumitem}\n", outputFile);
   if (preamble.containsImages)
      fputs("\\usepackage{graphicx}\n", outputFile);
   if (preamble.containsLinks)
      fputs("\\usepackage{hyperref}\n", outputFile);

   fputs("\\usepackage{textcomp}\n", outputFile);
   fputs("\\usepackage[hscale=0.73,vscale=0.82]{geometry}\n", outputFile);
   fputs("\\renewcommand*{\\familydefault}{\\sfdefault}\n\n", outputFile);

   if (preamble.title != NULL)
      fprintf(outputFile, "\\title{%s}\n", preamble.title);
   else
      fputs("\\title{}\n", outputFile);
   fputs("\\author{}\n", outputFile);
   fputs("\\date{}\n", outputFile);

   if (preamble.titleComments != NULL)
      fprintf(outputFile, "%s\n", preamble.titleComments);

   fputs("\n", outputFile);

   //copy file tmpBodyOutputFile to outputFile
   if (fseek(tmpBodyOutputFile, SEEK_SET, 0) != 0)
   {
      ERROR_MSG("insertPreamble", "Couldn't get to the beginning of the temporary output file");
      return RETURN_FAILURE;
   }

   char currentLine[FGETS_LINE_LENGTH];
   while(fgets(currentLine, FGETS_LINE_LENGTH, tmpBodyOutputFile) != NULL)
      fputs(currentLine, outputFile);

   return RETURN_SUCCESS;
}

void freePreamble()
{
   if (preamble.title != NULL)
      free(preamble.title);
}
