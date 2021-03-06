#include "compiler.h"

FILE* inputFile = NULL;
unsigned int currentLineNb = 0;
unsigned char nbAlinea = 0;
unsigned short inputIndentationNb = 0;
bool firstLineOfBodyInterpreted = false;

STATUS compile(const char* inputFileName, const char* outputFileName, bool keepTmpFile)
{
   FILE *outputFile = NULL, *tmpBodyOutputFile = NULL;

   //open input file
   inputFile = fopen(inputFileName, "r");
   if (inputFile == NULL)
   {
      char msg[256];
      snprintf(msg, 256, "%s '%s' %s", "Unable to open file", inputFileName, "(doesn't exist?)");
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
   if (translateLineByLine(tmpBodyOutputFile) != RETURN_SUCCESS)
   {
      fclose(inputFile);
      fclose(tmpBodyOutputFile);
      freePreamble();
      puts("Deleting temporary file...");
      deleteFile(tmpBodyOutputFilePath);
      free(tmpBodyOutputFilePath);
      return RETURN_FAILURE;
   }
   fclose(inputFile);

   //open output file
   outputFile = fopen(outputFileName, "w");
   if (outputFile == NULL)
   {
      ERROR_MSG("compile", "Couldn't open or create output file");
      freePreamble();
      fclose(tmpBodyOutputFile);
      deleteFile(tmpBodyOutputFilePath);
      free(tmpBodyOutputFilePath);
      return RETURN_FAILURE;
   }

   //write preamble to output file
   if (insertPreamble(tmpBodyOutputFile, outputFile) != RETURN_SUCCESS)
   {
      freePreamble();
      fclose(outputFile);
      fclose(tmpBodyOutputFile);
      deleteFile(tmpBodyOutputFilePath);
      free(tmpBodyOutputFilePath);
      return RETURN_FAILURE;
   }

   //close output file
   freePreamble();
   fclose(outputFile);

   //close temporary file
   fclose(tmpBodyOutputFile);
   if (!keepTmpFile)
   {
      puts("Deleting temporary tex file...");
      if (deleteFile(tmpBodyOutputFilePath) != RETURN_SUCCESS)
      {
         free(tmpBodyOutputFilePath);
         return RETURN_FAILURE;
      }
   }
   free(tmpBodyOutputFilePath);

   return RETURN_SUCCESS;
}

STATUS translateLineByLine(FILE* bodyOutputFile)
{
   fputs("\\begin{document}\n", bodyOutputFile);
   fputs("\\maketitle\n\n", bodyOutputFile);

   char* line;
   while ((line = getNextLineFromFile()) != NULL)
   {
      if (interpretLine(bodyOutputFile, &line[inputIndentationNb]) != RETURN_SUCCESS)
      {
         MD_WARNING(-1, "There was a problem translating lmd file to tex file. No tex file will be produced.");
         free(line);
         return RETURN_FAILURE;
      }
      free(line);
   }

   fputs("\\end{document}\n", bodyOutputFile);

   return RETURN_SUCCESS;
}

STATUS interpretLine(FILE* bodyOutputFile, const char* line)
{
   if (line[0] == '#')
   {
      char* partTitle = getTitleOfPart(line);
      char* translatedTitle = NULL;
      translateString(partTitle, &translatedTitle, true);
      removeUselessSpaces(translatedTitle);

      int commentIndex = getFirstIndexOfComment(line);

      if (line[1] != '#')//title
      {
         if (firstLineOfBodyInterpreted)
         {
            MD_WARNING(currentLineNb, "This line gets added to the title even though some lines which are not part of the title have already been written.");
         }
         addLineToTitle(translatedTitle);
         if (commentIndex >= 0)
            addCommentToTitle(&line[commentIndex]);

         nbAlinea = 1;
      }
      else if (line[2] != '#')//part
      {
         firstLineOfBodyInterpreted = true;

         if (commentIndex >= 0)
            fprintf(bodyOutputFile, "\t\\part{%s}%s\n\n", translatedTitle, &line[commentIndex]);
         else
            fprintf(bodyOutputFile, "\t\\part{%s}\n\n", translatedTitle);

         nbAlinea = 2;
      }
      else if (line[3] != '#')//section
      {
         firstLineOfBodyInterpreted = true;

         if (commentIndex >= 0)
            fprintf(bodyOutputFile, "\t\t\\section{%s}%s\n\n", translatedTitle, &line[commentIndex]);
         else
            fprintf(bodyOutputFile, "\t\t\\section{%s}\n\n", translatedTitle);

         nbAlinea = 3;
      }
      else if (line[4] != '#')//subsection
      {
         firstLineOfBodyInterpreted = true;

         if (commentIndex >= 0)
            fprintf(bodyOutputFile, "\t\t\t\\subsection{%s}%s\n\n", translatedTitle, &line[commentIndex]);
         else
            fprintf(bodyOutputFile, "\t\t\t\\subsection{%s}\n\n", translatedTitle);

         nbAlinea = 4;
      }
      else if (line[5] != '#')//subsubsection
      {
         firstLineOfBodyInterpreted = true;

         if (commentIndex >= 0)
            fprintf(bodyOutputFile, "\t\t\t\t\\subsubsection{%s}%s\n\n", translatedTitle, &line[commentIndex]);
         else
            fprintf(bodyOutputFile, "\t\t\t\t\\subsubsection{%s}\n\n", translatedTitle);

         nbAlinea = 5;
      }
      else if (line[6] != '#')//paragraph
      {
         firstLineOfBodyInterpreted = true;

         if (commentIndex >= 0)
            fprintf(bodyOutputFile, "\t\t\t\t\t\\paragraph{%s}%s\n", translatedTitle, &line[commentIndex]);
         else
            fprintf(bodyOutputFile, "\t\t\t\t\t\\paragraph{%s}\n", translatedTitle);

         nbAlinea = 6;
      }
      else//subparagraph
      {
         firstLineOfBodyInterpreted = true;

         if (commentIndex >= 0)
            fprintf(bodyOutputFile, "\t\t\t\t\t\t\\subparagraph{%s}%s\n", translatedTitle, &line[commentIndex]);
         else
            fprintf(bodyOutputFile, "\t\t\t\t\t\t\\subparagraph{%s}\n", translatedTitle);

         nbAlinea = 7;
      }

      free(partTitle);
      free(translatedTitle);
   }
   else if (isMultilinePlainTextOpeningTag(line))
   {
      firstLineOfBodyInterpreted = true;

      addVerbatimToPreamble();

      int beginningLineNb = currentLineNb;

      char* nextLine = getNextLineFromFile();

      fputs("\\begin{spverbatim}\n", bodyOutputFile);

      while (!isMultilinePlainTextClosingTag(nextLine))
      {
         int firstUsefulPartIndex;
         for (firstUsefulPartIndex=0; nextLine[firstUsefulPartIndex]==' '||nextLine[firstUsefulPartIndex]=='\t'; firstUsefulPartIndex++)
            ;//remove alinea

         for (int i=firstUsefulPartIndex; nextLine[i]!='\0'; i++)
            fputc(nextLine[i], bodyOutputFile);
         fputc('\n', bodyOutputFile);

         free(nextLine);
         nextLine = getNextLineFromFile();
         if (nextLine == NULL)
            break;
      }
      if (nextLine != NULL)
         free(nextLine);
      else
         MD_WARNING(beginningLineNb, "Missing closing tag for multiline plain text (])");

      fputs("\\end{spverbatim}\n", bodyOutputFile);
   }
   else if (isImageLine(line))
   {
      firstLineOfBodyInterpreted = true;

      addImagesToPreamble();

      if (strchr(line, '>') == NULL)
      {
         MD_WARNING(currentLineNb, "Missing closing tag for image line '>'. Tex file could be wrong.");
      }

      writeAlinea(bodyOutputFile);
      fputs("\\begin{figure}[h!]\n", bodyOutputFile);
      nbAlinea++;

      writeAlinea(bodyOutputFile);
      fputs("\\centering\n", bodyOutputFile);

      char* imageFileName = pickImageFileName(line);
      if (imageFileName != NULL)
      {
         if (!fileExists(imageFileName))
         {
            char msg[512];
            snprintf(msg, 512, "Couldn't find file '%s'", imageFileName);
            MD_ERROR(currentLineNb, msg);
            free(imageFileName);
            return RETURN_FAILURE;
         }

         if (!containsImageSize(line))
         {
            writeAlinea(bodyOutputFile);
            fprintf(bodyOutputFile, "\\includegraphics{%s}\n", imageFileName);
            free(imageFileName);
         }
         else
         {
            if (containsImageScale(line))
            {
               writeAlinea(bodyOutputFile);

               double scale = getImageScale(line);
               if (scale == -1.0)
                  fprintf(bodyOutputFile, "\\includegraphics{%s}\n", imageFileName);
               else
                  fprintf(bodyOutputFile, "\\includegraphics[scale=%f]{%s}\n", scale, imageFileName);
            }
            else
            {
               char *widthString = getImageWidth(line), *heightString = getImageHeight(line);
               writeAlinea(bodyOutputFile);

               if (widthString == NULL || heightString == NULL)
               {
                  if (widthString == NULL && heightString == NULL)
                     fprintf(bodyOutputFile, "\\includegraphics{%s}\n", imageFileName);
                  else if (widthString == NULL)
                     fprintf(bodyOutputFile, "\\includegraphics[height=%s]{%s}\n", heightString, imageFileName);
                  else
                     fprintf(bodyOutputFile, "\\includegraphics[width=%s]{%s}\n", widthString, imageFileName);
               }
               else
               {
                  if (strcmp(widthString, "/") == 0 && strcmp(heightString, "/") == 0)
                     fprintf(bodyOutputFile, "\\includegraphics{%s}\n", imageFileName);
                  else if (strcmp(widthString, "/") == 0)
                     fprintf(bodyOutputFile, "\\includegraphics[height=%s]{%s}\n", heightString, imageFileName);
                  else if (strcmp(heightString, "/") == 0)
                     fprintf(bodyOutputFile, "\\includegraphics[width=%s]{%s}\n", widthString, imageFileName);
                  else
                     fprintf(bodyOutputFile, "\\includegraphics[width=%s,height=%s]{%s}\n", widthString, heightString, imageFileName);
               }

               free(widthString);
               free(heightString);
            }
         }

         char* label = pickImageLabel(line);
         char* translatedLabel;
         if (label != NULL)
         {
            translateString(label, &translatedLabel, false);

            writeAlinea(bodyOutputFile);
            fprintf(bodyOutputFile, "\\caption{%s}\n", translatedLabel);

            free(label);
            free(translatedLabel);
         }
      }
      else
         MD_ERROR(currentLineNb, "Couldn't include image.");

      nbAlinea--;
      writeAlinea(bodyOutputFile);
      fputs("\\end{figure}\n\n", bodyOutputFile);
   }
   else if (isItemizeLine(line))
   {
      firstLineOfBodyInterpreted = true;

      addEnumToPreamble();
      writeItemize(bodyOutputFile, line);
      fputc('\n', bodyOutputFile);
   }
   else if (isEnumLine(line))
   {
      firstLineOfBodyInterpreted = true;

      addEnumToPreamble();
      writeEnumerate(bodyOutputFile, line);
      fputc('\n', bodyOutputFile);
   }
   else if (isSimpleTableTagLine(line))
   {
      firstLineOfBodyInterpreted = true;

      writeSimpleTable(bodyOutputFile);
   }
   else
   {
      firstLineOfBodyInterpreted = true;

      writeAlinea(bodyOutputFile);
      translateToFile(bodyOutputFile, line);
      fputc('\n', bodyOutputFile);
      fputc('\n', bodyOutputFile);//to have a new line in the pdf document
   }

   return RETURN_SUCCESS;
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

   //update reading informations
   currentLineNb++;
   inputIndentationNb = getIndentation(line);

   return line;
}

unsigned short getIndentation(const char* line)
{
   unsigned short nbFirstSpaces;
   for (nbFirstSpaces=0; line[nbFirstSpaces]==' ' || line[nbFirstSpaces]=='\t'; nbFirstSpaces++)
      ;

   return nbFirstSpaces;
}

char* getTitleOfPart(const char* line)
{
   int firstTitleCharIndex;
   for (firstTitleCharIndex=1; line[firstTitleCharIndex]=='#' || line[firstTitleCharIndex]==' ' || line[firstTitleCharIndex]=='\t'; firstTitleCharIndex++)
      ;

   char* tmp = malloc( strlen(line)*sizeof(char) );
   int iDest = 0;
   for (int i=firstTitleCharIndex; line[i]!='%' && line[i]!='\0'; i++, iDest++)
   {
      tmp[iDest] = line[i];
      if (line[i] == '\\')
      {
         i++;
         iDest++;
         tmp[iDest] = line[i];
      }
   }
   tmp[iDest] = '\0';

   char* answer = malloc( (strlen(tmp)+1)*sizeof(char) );
   strcpy(answer, tmp);
   free(tmp);

   return answer;
}

int getFirstIndexOfComment(const char* line)
{
   if (line[0]=='%')
      return 0;
   for (int i=1; line[i]!='\0'; i++)
   {
      if (line[i]=='%' && line[i-1]!='\\')
         return i;
   }
   return -1;
}

void removeUselessSpaces(char* string)
{
   if (string[0] == ' ' || string[0] == '\t')//should never be true
      string = &string[inputIndentationNb];

   int lastCharIndex = strlen(string)-1;
   while (string[lastCharIndex]==' ' || string[lastCharIndex]=='\t')
   {
      string[lastCharIndex] = '\0';
      lastCharIndex = strlen(string)-1;
      if (lastCharIndex <= -1)
         break;
   }
}

bool isMultilinePlainTextOpeningTag(const char* line)
{
   if (line[0]!='[')
      return false;

   int i;
   for (i=1; line[i]==' ' || line[i]=='\t'; i++)
      ;

   if (line[i]!='\0' && line[i]!='%')
      return false;

   return true;
}

bool isMultilinePlainTextClosingTag(const char* line)
{
   int firstUsefulPartIndex;
   for (firstUsefulPartIndex=0; line[firstUsefulPartIndex]==' ' || line[firstUsefulPartIndex]=='\t'; firstUsefulPartIndex++)
      ;

   if (line[firstUsefulPartIndex]!=']')
      return false;

   int i;
   for (i=firstUsefulPartIndex+1; line[i]==' ' || line[i]=='\t'; i++)
      ;

   if (line[i]!='\0' && line[i]!='%')
      return false;

   return true;
}

bool isImageLine(const char* line)
{
   //if line begins in "<img " or "<img\t"
   if (line[0]=='<' && line[1]=='i' && line[2]=='m' && line[3]=='g')
      return (line[4]==' ' || line[4]=='\t');
   return false;
}

void writeAlinea(FILE* file)
{
   for (int i=0; i<nbAlinea; i++)
      fputc('\t', file);
}

char* pickImageFileName(const char* line)
{
   char* tmp = malloc(strlen(line)*sizeof(char));

   int firstNameIndex;
   for (firstNameIndex=4; line[firstNameIndex]==' ' || line[firstNameIndex]=='\t'; firstNameIndex++)
      ;

   int iDest=0;
   for (int i=firstNameIndex; line[i]!='>' && line[i]!=' ' && line[i]!='\t' && line[i]!='\0'; i++, iDest++)
      tmp[iDest] = line[i];
   tmp[iDest] = '\0';

   char* imageFileName = malloc( (strlen(tmp)+1)*sizeof(char) )   ;
   strcpy(imageFileName, tmp);
   free(tmp);

   return imageFileName;
}

char* pickImageLabel(const char* line)
{
   int firstNameIndex;
   for (firstNameIndex=4; line[firstNameIndex]==' ' || line[firstNameIndex]=='\t'; firstNameIndex++)
      ;

   int lastNameIndex;
   for (lastNameIndex=firstNameIndex; line[lastNameIndex]!='>' && line[lastNameIndex]!=' ' && line[lastNameIndex]!='\t' && line[lastNameIndex]!='\0'; lastNameIndex++)
      ;

   if (line[lastNameIndex]=='>')
      return NULL;
   if (line[lastNameIndex]=='\0')
   {
      MD_WARNING(currentLineNb, "Missing closing tag for image inclusion (>)");
      return NULL;
   }

   int firstLabelIndex;
   for (firstLabelIndex=lastNameIndex; line[firstLabelIndex]==' '||line[firstLabelIndex]=='\t'; firstLabelIndex++)
      ;

   char* tmp = malloc(strlen(line)*sizeof(char));

   int iDest = 0;
   for (int i=firstLabelIndex; line[i]!='>' && line[i]!='|' && line[i]!='\0'; i++, iDest++)
   {
      if (line[i] == '\\' && line[i+1] != '\\')
         i++;
      tmp[iDest] = line[i];
   }
   tmp[iDest] = '\0';

   char* label = malloc( (strlen(tmp)+1)*sizeof(char) );
   strcpy(label, tmp);
   free(tmp);

   return label;
}

bool containsImageSize(const char* line)
{
   if (strchr(line, '|') == NULL)
      return false;

   if (strchr(line, '%') == NULL)
      return true;

   for (unsigned int i=0; line[i]!='>' && line[i]!='\0'; i++)
   {
      if (line[i] == '|')
         return true;
      if (line[i] == '\\')
         i++;
   }
   return false;
}

bool containsImageScale(const char* line)
{
   if (strchr(line, ':') == NULL)
      return true;

   if (strchr(line, '%') == NULL)
      return false;

   char* pointerToPipe = getPointerToImageSizeTag(line);
   for (unsigned int i=0; pointerToPipe[i]!='>' && pointerToPipe[i]!='\0'; i++)
   {
      if (pointerToPipe[i] == ':')
         return false;
   }
   return true;
}

double getImageScale(const char* line)
{
   char* pointerToPipe = getPointerToImageSizeTag(line);

   unsigned short firstScaleIndex;
   for (firstScaleIndex=1; pointerToPipe[firstScaleIndex]==' ' || pointerToPipe[firstScaleIndex]=='\t'; firstScaleIndex++)
      ;

   char* tmp = malloc(strlen(line)*sizeof(char));
   unsigned short iDest = 0;
   for (unsigned short i=firstScaleIndex; pointerToPipe[i]!=' ' && pointerToPipe[i]!='>' && pointerToPipe[i]!='\0'; i++, iDest++)
      tmp[iDest] = pointerToPipe[i];
   tmp[iDest] = '\0';

   char* pointerToComma = strchr(tmp, ',');
   if (pointerToComma != NULL)
      *pointerToComma = '.';

   double scale = strtod(tmp, NULL);
   free(tmp);
   if (scale == 0.0)
   {
      MD_ERROR(currentLineNb, "Scale of image must be a real number.");
      return -1.0;
   }

   return scale;
}

char* getImageWidth(const char* line)
{
   char* pointerToPipe = getPointerToImageSizeTag(line);

   unsigned short firstWidthIndex;
   for (firstWidthIndex=1; pointerToPipe[firstWidthIndex]==' ' || pointerToPipe[firstWidthIndex]=='\t'; firstWidthIndex++)
      ;

   char* tmp = malloc(strlen(line)*sizeof(char));
   unsigned short iDest = 0;
   for (unsigned short i=firstWidthIndex; pointerToPipe[i]!=':'; i++, iDest++)
      tmp[iDest] = pointerToPipe[i];
   tmp[iDest] = '\0';

   if (strchr(tmp, '/') != NULL)
   {
      free(tmp);

      char* answer = malloc(2*sizeof(char));
      answer[0] = '/';
      answer[1] = '\0';

      return answer;
   }

   char* endPtr = NULL;
   long width = strtol(tmp, &endPtr, 10);
   if (endPtr == tmp)
   {
      MD_ERROR(currentLineNb, "Image width must be a number and a unit.");
      free(tmp);
      return NULL;
   }
   if (width <= 0)
   {
      MD_WARNING(currentLineNb, "Image width should be greater than 0. Turning it into positive number.");
      width *= -1;
   }

   char* widthString = malloc( (getNbLength(width)+3)*sizeof(char) );

   if (strstr(tmp, "cm") != NULL)
      sprintf(widthString, "%licm", width);
   else if (strstr(tmp, "em") != NULL)
      sprintf(widthString, "%liem", width);
   else if (strstr(tmp, "pt") != NULL)
      sprintf(widthString, "%lipt", width);
   else if (strstr(tmp, "mm") != NULL)
      sprintf(widthString, "%limm", width);
   else if (strstr(tmp, "ex") != NULL)
      sprintf(widthString, "%liex", width);
   else if (strstr(tmp, "in") != NULL)
      sprintf(widthString, "%liin", width);
   else
   {
      MD_WARNING(currentLineNb, "Image width has not a correct unit (cm, mm, pt, in, em or ex). Turning it into centimeters.");
      sprintf(widthString, "%licm", width);
   }

   free(tmp);

   return widthString;
}

char* getImageHeight(const char* line)
{
   char* pointerToColon = strchr(line, ':');

   unsigned short firstHeightIndex;
   for (firstHeightIndex=1; pointerToColon[firstHeightIndex]==' ' && pointerToColon[firstHeightIndex]=='\t'; firstHeightIndex++)
      ;

   char* tmp = malloc(strlen(line)*sizeof(char));
   unsigned short iDest = 0;
   for (unsigned short i=firstHeightIndex; pointerToColon[i]!='>' && pointerToColon[i]!='%' && pointerToColon[i]!='\0'; i++, iDest++)
      tmp[iDest] = pointerToColon[i];
   tmp[iDest] = '\0';

   if (strchr(tmp, '/') != NULL)
   {
      free(tmp);

      char* answer = malloc(2*sizeof(char));
      answer[0] = '/';
      answer[1] = '\0';

      return answer;
   }

   char* endPtr = NULL;
   long height = strtol(tmp, &endPtr, 10);
   if (endPtr == tmp)
   {
      MD_ERROR(currentLineNb, "Image height must be a number and a unit");
      free(tmp);
      return NULL;
   }
   if (height <= 0)
   {
      MD_WARNING(currentLineNb, "Image height must be greater than 0. Turning it into a positive number.");
      height *= -1;
   }

   char* heightString = malloc( (getNbLength(height)+3)*sizeof(char) );

   if (strstr(tmp, "cm") != NULL)
      sprintf(heightString, "%licm", height);
   else if (strstr(tmp, "em") != NULL)
      sprintf(heightString, "%liem", height);
   else if (strstr(tmp, "pt") != NULL)
      sprintf(heightString, "%lipt", height);
   else if (strstr(tmp, "mm") != NULL)
      sprintf(heightString, "%limm", height);
   else if (strstr(tmp, "ex") != NULL)
      sprintf(heightString, "%liex", height);
   else if (strstr(tmp, "in") != NULL)
      sprintf(heightString, "%liin", height);
   else
   {
      MD_WARNING(currentLineNb, "Image height has not a correct unit (cm, mm, pt, in, em or ex). Turning it into centimeters.");
      sprintf(heightString, "%licm", height);
   }

   free(tmp);

   return heightString;
}

char* getPointerToImageSizeTag(const char* line)
{
   for (unsigned int i=0; line[i]!='\0' && line[i]!='>' && line[i]!='%'; i++)
   {
      if (line[i] == '|')
         return (char*) &line[i];
      if (line[i] == '\\')
         i++;
   }
   return NULL;
}

unsigned int getNbLength(long nb)
{
   for (unsigned int i=0, divisor=1; ; i++, divisor*=10)
   {
      if (nb / divisor == 0)
         return i;
   }
}

char* pickURL(const char* line, unsigned int firstURLIndex)
{
   if (line[firstURLIndex]!='h')
   {
      MD_ERROR(currentLineNb, "There is an invalid link in this line\nIf it is not supposed to be a link, write '\\<' instead of '<'");
      return NULL;
   }
   char* tmp = malloc(strlen(line)*sizeof(char));

   int iDest = 0;
   for (int i=firstURLIndex; line[i]!='>' && line[i]!=' ' && line[i]!='\t' && line[i]!='\0'; i++, iDest++)
      tmp[iDest] = line[i];
   tmp[iDest] = '\0';

   char* url = malloc( (strlen(tmp)+1)*sizeof(char) );
   strcpy(url, tmp);
   free(tmp);

   return url;
}

char* pickURLLabel(const char* line, unsigned int firstURLIndex)
{
   int firstSpaceIndex;
   for (firstSpaceIndex=firstURLIndex; line[firstSpaceIndex]!='>' && line[firstSpaceIndex]!=' ' && line[firstSpaceIndex]!='\t' && line[firstSpaceIndex]!='\0'; firstSpaceIndex++)
      ;

   if (line[firstSpaceIndex]=='\0')
   {
      MD_ERROR(currentLineNb, "Missing closing tag for hyperlink (>)");
      return NULL;
   }
   if (line[firstSpaceIndex]=='>')
      return NULL;

   //line[firstSpaceIndex]==' ' || line[firstSpaceIndex]=='\t'
   int firstLabelIndex;
   for (firstLabelIndex=firstSpaceIndex; line[firstLabelIndex]==' ' || line[firstLabelIndex]=='\t'; firstLabelIndex++)
      ;

   char* tmp = malloc(strlen(line)*sizeof(char));

   int i, iDest = 0;
   for (i=firstLabelIndex; line[i]!='>' && line[i]!='\0'; i++, iDest++)
      tmp[iDest] = line[i];
   tmp[iDest] = '\0';

   removeUselessSpaces(tmp);

   if (line[i] == '\0')
   {
      MD_WARNING(currentLineNb, "Missing closing tag for hyperlink (>)");
   }

   char* label = malloc( (strlen(tmp)+1)*sizeof(char) );
   strcpy(label, tmp);
   free(tmp);

   return label;
}

bool isItemizeLine(const char* line)
{
   int firstNonSpaceIndex;
   for (firstNonSpaceIndex=0; line[firstNonSpaceIndex]==' ' || line[firstNonSpaceIndex]=='\t'; firstNonSpaceIndex++)
      ;

   if (line[firstNonSpaceIndex]=='+')
      return (line[firstNonSpaceIndex+1]==' ' || line[firstNonSpaceIndex+1]=='\t');
   return false;
}

char* pickItemFromItemize(const char* line)
{
   int firstNonSpaceIndex;
   for (firstNonSpaceIndex=0; line[firstNonSpaceIndex]==' ' || line[firstNonSpaceIndex]=='\t'; firstNonSpaceIndex++)
      ;

   if (line[firstNonSpaceIndex]!='+')
   {
      ERROR_MSG("pickItemFromItemize", "Tried to pick item from line that doesn't begin with '+'");
      return NULL;
   }

   char* tmp = malloc(strlen(line)*sizeof(char));

   int firstItemIndex;
   for (firstItemIndex=firstNonSpaceIndex+1; line[firstItemIndex]==' ' || line[firstItemIndex]=='\t'; firstItemIndex++)
      ;

   int iDest = 0;
   for (int i=firstItemIndex; line[i]!='\0'; i++, iDest++)
      tmp[iDest] = line[i];
   tmp[iDest] = '\0';

   char* item = malloc( (strlen(tmp)+1)*sizeof(char) );
   strcpy(item, tmp);
   free(tmp);
   return item;
}

void writeItemize(FILE* bodyOutputFile, const char* line)
{
   unsigned char itemizeIndentation = inputIndentationNb;

   writeAlinea(bodyOutputFile);
   fputs("\\begin{itemize}[label=$\\bullet$]\n", bodyOutputFile);
   nbAlinea++;

   writeItemizeItem(bodyOutputFile, line);

   char* nextLine = getNextLineFromFile();
   while (nextLine != NULL && (isItemizeLine(nextLine) || isEnumLine(nextLine)))
   {
      if (isItemizeLine(nextLine))
      {
         if (inputIndentationNb == itemizeIndentation)
            writeItemizeItem(bodyOutputFile, nextLine);
         else if (inputIndentationNb > itemizeIndentation)
         {
            nbAlinea++;
            writeItemize(bodyOutputFile, nextLine);
            nbAlinea--;
         }
         else//back to parent itemize
            break;
      }
      else//isEnumLine(nextLine)
      {
         if (inputIndentationNb == itemizeIndentation)
         {
            MD_WARNING(currentLineNb, "Wrong indentation for enumerate. Enumerate must not be indented on the same level as the itemize environment");
            nbAlinea++;
            writeEnumerate(bodyOutputFile, nextLine);
            nbAlinea--;
         }
         else if (inputIndentationNb > itemizeIndentation)
         {
            nbAlinea++;
            writeEnumerate(bodyOutputFile, nextLine);
            nbAlinea--;
         }
         else//back to parent enumerate
            break;
      }

      free(nextLine);
      nextLine = getNextLineFromFile();
   }

   nbAlinea--;
   writeAlinea(bodyOutputFile);
   fputs("\\end{itemize}\n", bodyOutputFile);

   if (nextLine != NULL)
   {
      //put cursor back to beginning of line
      fseek(inputFile, -(strlen(nextLine)+1), SEEK_CUR);
      currentLineNb--;
      free(nextLine);
   }
}

void writeItemizeItem(FILE* bodyOutputFile, const char* line)
{
   char* item = pickItemFromItemize(line);
   if (item == NULL)
      return;
   char* translatedItem;
   translateString(item, &translatedItem, false);

   writeAlinea(bodyOutputFile);
   fprintf(bodyOutputFile, "\\item %s\n", translatedItem);

   free(item);
   free(translatedItem);
}

bool isEnumLine(const char* line)
{
   unsigned int firstNonSpaceIndex;
   for (firstNonSpaceIndex=0; line[firstNonSpaceIndex]==' ' || line[firstNonSpaceIndex]=='\t'; firstNonSpaceIndex++)
      ;

   if (line[firstNonSpaceIndex] != 'l')
      return false;

   if (line[firstNonSpaceIndex+1] != '.')
      return false;
   return (line[firstNonSpaceIndex+2] == ' ' || line[firstNonSpaceIndex]=='\t');
}

void writeEnumerate(FILE* bodyOutputFile, const char* line)
{
   unsigned char enumerateIndentation = inputIndentationNb;

   writeAlinea(bodyOutputFile);
   fputs("\\begin{enumerate}\n", bodyOutputFile);
   nbAlinea++;

   writeEnumerateItem(bodyOutputFile, line);

   char* nextLine = getNextLineFromFile();
   while (nextLine!=NULL && (isEnumLine(nextLine) || isItemizeLine(nextLine)))
   {
      if (isEnumLine(nextLine))
      {
         if (inputIndentationNb == enumerateIndentation)
            writeEnumerateItem(bodyOutputFile, nextLine);
         else if (inputIndentationNb > enumerateIndentation)
         {
            nbAlinea++;
            writeEnumerate(bodyOutputFile, nextLine);
            nbAlinea--;
         }
         else//back to parent enumerate
            break;
      }
      else//isItemizeLine(nextLine)
      {
         if (inputIndentationNb == enumerateIndentation)
         {
            MD_WARNING(currentLineNb, "Wrong indentation for itemize environment. The itemize must not be indented on the same level as the enumerate environment");
            nbAlinea++;
            writeItemize(bodyOutputFile, nextLine);
            nbAlinea--;
         }
         else if (inputIndentationNb > enumerateIndentation)
         {
            nbAlinea++;
            writeItemize(bodyOutputFile, nextLine);
            nbAlinea--;
         }
         else//back to parent itemize
            break;
      }

      free(nextLine);
      nextLine = getNextLineFromFile();
   }

   if (nextLine != NULL)//put cursor back to beginning of the line
   {
      fseek(inputFile, -(strlen(nextLine)+1), SEEK_CUR);
      free(nextLine);
      currentLineNb--;
   }

   nbAlinea--;
   writeAlinea(bodyOutputFile);
   fputs("\\end{enumerate}\n", bodyOutputFile);
}

void writeEnumerateItem(FILE* bodyOutputFile, const char* line)
{
   char* item = pickItemFromEnumerate(line);
   if (item == NULL)
      return;
   char* translatedItem;
   translateString(item, &translatedItem, false);

   writeAlinea(bodyOutputFile);
   fprintf(bodyOutputFile, "\\item %s\n", translatedItem);

   free(item);
   free(translatedItem);
}

char* pickItemFromEnumerate(const char* line)
{
   unsigned int firstNonSpaceIndex;
   for (firstNonSpaceIndex=0; line[firstNonSpaceIndex]==' ' || line[firstNonSpaceIndex]=='\t'; firstNonSpaceIndex++)
      ;

   if (line[firstNonSpaceIndex]!='l')
   {
      ERROR_MSG("pickItemFromEnumerate", "Tried to pick item from no enumerate section");
      return NULL;
   }
   if (line[firstNonSpaceIndex+1]!='.')
   {
      ERROR_MSG("pickItemFromEnumerate", "Tried to pick item from line that doesn't begin with \"l. \" or \"l.\\t\"");
      return NULL;
   }

   unsigned int firstItemIndex;
   for (firstItemIndex=firstNonSpaceIndex+2; line[firstItemIndex]==' ' || line[firstItemIndex]=='\t'; firstItemIndex++)
      ;

   char* tmp = malloc(strlen(line)*sizeof(char));

   unsigned int iDest = 0;
   for (unsigned int i=firstItemIndex; line[i]!='\0'; i++, iDest++)
      tmp[iDest] = line[i];
   tmp[iDest] = '\0';

   char* item = malloc( (strlen(tmp)+1)*sizeof(char) );
   strcpy(item, tmp);
   free(tmp);

   return item;
}

bool isSimpleTableTagLine(const char* line)
{
   unsigned short firstNonSpaceIndex = 0;
   if (line[0]==' ' || line[0]=='\t')
      firstNonSpaceIndex = inputIndentationNb;

   if (line[firstNonSpaceIndex]!='=')
      return false;

   unsigned short i;
   for (i=firstNonSpaceIndex; line[i]=='='; i++)
      ;

   if (line[i]!='\0' && line[i]!='%' && line[i]!=' ' && line[i]!='\t')
      return false;

   if (line[i]==' ' || line[i]=='\t')
   {
      for (;line[i]==' ' || line[i]=='\t'; i++)
         ;
      if (line[i]!='\0' && line[i]!='%')
         return false;
   }

   return true;
}

void writeSimpleTable(FILE* bodyOutputFile)
{
   char* nextLine = getNextLineFromFile();
   unsigned char nbColumns = getNbSimpleTableColumns(nextLine);
   if (nbColumns == 0)
   {
      //write line to output file
      fputs("=============\n\n", bodyOutputFile);
      //back to beginning of the line
      fseek(inputFile, -(strlen(nextLine)+1), SEEK_CUR);
      currentLineNb--;
      free(nextLine);
      MD_WARNING(currentLineNb, "This line is not a table line (must begin in '|'), even though the last line was a table opening line. The line with equal signs could be wrongly translated. Use '\\' at the beginning of the line if it isn't supposed to be interpreted as a table.");
      return;
   }

   writeAlinea(bodyOutputFile);
   fputs("\\begin{tabular}{", bodyOutputFile);
   for (int i=0; i<nbColumns; i++)
      fputs("|l", bodyOutputFile);
   fputs("|}\n", bodyOutputFile);
   nbAlinea++;

   writeAlinea(bodyOutputFile);
   fputs("\\hline\n", bodyOutputFile);

   char *currentCellContent = NULL, *currentTranslatedCellContent;
   while (nextLine != NULL && !isSimpleTableTagLine(nextLine))
   {
      if (isSimpleTableCellsLine(nextLine))
      {
         for (int i=0; i<nbColumns; i++)
         {
            currentCellContent = getSimpleTableCellContent(nextLine, i);
            if (currentCellContent == NULL)
            {
               MD_ERROR(currentLineNb, "Wrong number of columns on that line.");
               free(currentCellContent);
               break;
            }

            translateString(currentCellContent, &currentTranslatedCellContent, false);
            //TODO translate cell
            if (i == 0)
            {
               writeAlinea(bodyOutputFile);
               fprintf(bodyOutputFile, "%s", currentTranslatedCellContent);
            }
            else
               fprintf(bodyOutputFile, " & %s", currentTranslatedCellContent);

            free(currentTranslatedCellContent);
            free(currentCellContent);
         }
         fputs("\\\\\n", bodyOutputFile);
      }
      else if (isSimpleTableHorizontalLine(nextLine))
      {
         writeAlinea(bodyOutputFile);
         fputs("\\hline\n", bodyOutputFile);
      }
      else
      {
         MD_ERROR(currentLineNb, "This line is not a valid table cells line (must begin in '|'). It will not be translated.");
      }

      free(nextLine);
      nextLine = getNextLineFromFile();
   }

   if (nextLine != NULL)
      free(nextLine);
   else
   {
      MD_ERROR(currentLineNb, "Missing closing table line.");
   }

   writeAlinea(bodyOutputFile);
   fputs("\\hline\n", bodyOutputFile);

   nbAlinea--;
   writeAlinea(bodyOutputFile);
   fputs("\\end{tabular}\n\n", bodyOutputFile);
}

bool isSimpleTableCellsLine(const char* line)
{
   unsigned short firstNonSpaceIndex = 0;
   if (line[0] == ' ' || line[0] == '\t')
      firstNonSpaceIndex = inputIndentationNb;

   return (line[firstNonSpaceIndex] == '|');
}

unsigned char getNbSimpleTableColumns(const char* line)
{
   int firstNonSpaceIndex = 0;
   if (line[0]==' ' || line[0]=='\t')
      firstNonSpaceIndex = inputIndentationNb;

   if (line[firstNonSpaceIndex] != '|')
      return 0;

   unsigned char nbColumns = 0;
   int i = firstNonSpaceIndex+1;
   while (line[i]!='\0' && line[i]!='%')
   {
      if (line[i] == '|')
         nbColumns++;
      else if (line[i] == '\\' && line[i+1] != '\0')
         i++;

      i++;
   }

   return nbColumns;
}

char* getSimpleTableCellContent(const char* line, unsigned short index)
{
   unsigned short firstNonSpaceIndex = 0;
   if (line[0]==' ' || line[0]=='\t')
      firstNonSpaceIndex = inputIndentationNb;

   unsigned short firstRequestedCellTextIndex = firstNonSpaceIndex;
   for (int i=0; i<=index; i++)
   {
      for (; line[firstRequestedCellTextIndex]!='|' && line[firstRequestedCellTextIndex]!='\0'; firstRequestedCellTextIndex++)
         ;

      if (line[firstRequestedCellTextIndex] == '\0')
      {
         MD_ERROR(currentLineNb, "Missing last vertical line '|'.");
         return NULL;
      }
      firstRequestedCellTextIndex++;
   }

   char* tmp = malloc(strlen(line)*sizeof(char));

   unsigned short i, iDest = 0;
   for (i=firstRequestedCellTextIndex; line[i]!='|' && line[i]!='\0' && line[i]!='%'; i++, iDest++)
      tmp[iDest] = line[i];
   tmp[iDest] = '\0';

   if (line[i] == '\0')
   {
      MD_WARNING(currentLineNb, "Missing last vertical line '|'.");
   }

   removeUselessSpaces(tmp);

   char* cellContent = malloc( (strlen(tmp)+1)*sizeof(char) );
   strcpy(cellContent, tmp);
   free(tmp);

   return cellContent;
}

bool isSimpleTableHorizontalLine(const char* line)
{
   unsigned short firstNonSpaceIndex = 0;
   if (line[0]==' ' || line[0]=='\t')
      firstNonSpaceIndex = inputIndentationNb;

   if (line[firstNonSpaceIndex]!='-')
      return false;

   unsigned short i;
   for (i=firstNonSpaceIndex; line[i]=='-'; i++)
      ;

   if (line[i]!='\0' && line[i]!='%' && line[i]!=' ' && line[i]!='\t')
      return false;

   if (line[i]==' ' || line[i]=='\t')
   {
      for (;line[i]==' ' || line[i]=='\t'; i++)
         ;
      if (line[i]!='\0' && line[i]!='%')
         return false;
   }

   return true;
}

void translateString(const char* source, char** destination, bool isTitle)
{
	int translatedStringLength = strlen(source)+1;
	char* translatedString = malloc(translatedStringLength*sizeof(char));
	translatedString[0] = '\0';

	Pile environments;
	environments.top = NULL;

	int i, iTranslated;
	for (i=0, iTranslated = 0; source[i] != '\0'; i++, iTranslated++)
	{
      if (!isTitle && getTop(&environments) == PLAIN_TEXT && source[i] != ']')
      {
         translatedString[iTranslated] = source[i];
         continue;
      }

		switch (source[i])
		{
			case '\\':
				switch (source[i+1])
				{
					case '\\':
						translatedStringLength += 13;
						reallocate(&translatedString, translatedStringLength);
                  translatedString[iTranslated] = '\0';
						sprintf(translatedString, "%s\\textbackslash ", translatedString);
						iTranslated += 14;
						i++;
						break;
					case '%':
						translatedString[iTranslated] = '\\';
						translatedString[iTranslated+1] = '%';
						iTranslated++;
						i++;
						break;
					case '_':
						translatedString[iTranslated] = '\\';
						translatedString[iTranslated+1] = '_';
						iTranslated++;
						i++;
						break;
					case '#':
						translatedString[iTranslated] = '\\';
						translatedString[iTranslated] = '#';
						iTranslated++;
						i++;
						break;
					case '~':
						translatedStringLength += 4;
						reallocate(&translatedString, translatedStringLength);
                  translatedString[iTranslated] = '\0';
						sprintf(translatedString, "%s$\\sim$", translatedString);
						iTranslated += 5;
						i++;
						break;
					case '.':
						translatedString[iTranslated] = '.';
						i++;
						break;
					case '*':
						translatedString[iTranslated] = '*';
						i++;
						break;
					case '[':
						translatedString[iTranslated] = '[';
						i++;
						break;
					case ']':
						translatedString[iTranslated] = ']';
						i++;
						break;
					case '+':
						translatedString[iTranslated] = '+';
						i++;
						break;
               case '=':
                  translatedString[iTranslated] = '=';
                  i++;
                  break;
					default:
						translatedString[iTranslated] = '\\';
						break;
				}
				break;
			case '*'://TODO fix embedded BOLD and ITALIC environments
				if (getTop(&environments) == BOLD)
				{
					if (source[i+1] == '*')
					{
						//closing BOLD environment
						pilePop(&environments);
						translatedString[iTranslated] = '}';//there's already a place allocated (allocated when the environment was created)
						i++;
					}
					else
					{
						//opening ITALIC environment
						pilePush(&environments, ITALIC);
						translatedStringLength += 7;//already adding a place for the closing '}'
						reallocate(&translatedString, translatedStringLength);
                  translatedString[iTranslated] = '\0';
						sprintf(translatedString, "%s\\textit{", translatedString);
						iTranslated += 7;
					}
				}
				else if (getTop(&environments) == ITALIC)
				{
					//closing ITALIC environment
					pilePop(&environments);
					translatedString[iTranslated] = '}';//there's already a place allocated (allocated when the environment was created)
				}
				else
				{
					if (source[i+1] == '*')
					{
						//opening BOLD environment
						pilePush(&environments, BOLD);
						translatedStringLength += 5;//already adding a place for the closing '}'
						reallocate(&translatedString, translatedStringLength);
                  translatedString[iTranslated] = '\0';
						sprintf(translatedString, "%s\\textbf{", translatedString);
						iTranslated += 7;
						i++;
					}
					else
					{
						//opening ITALIC environment
						pilePush(&environments, ITALIC);
						translatedStringLength += 7;//already adding a place for the closing '}'
						reallocate(&translatedString, translatedStringLength);
                  translatedString[iTranslated] = '\0';
						sprintf(translatedString, "%s\\textit{", translatedString);
						iTranslated += 7;
					}
				}
				break;
			case '_':
				if (getTop(&environments) == UNDERLINE)
				{
					//closing UNDERLINE environment
					pilePop(&environments);
					translatedString[iTranslated] = '}';
				}
				else
				{
					//opening UNDERLINE environment
					pilePush(&environments, UNDERLINE);
					translatedStringLength += 10;
					reallocate(&translatedString, translatedStringLength);
               translatedString[iTranslated] = '\0';
					sprintf(translatedString, "%s\\underline{", translatedString);
					iTranslated += 10;
				}
				break;
			case '~':
				if (getTop(&environments) == STRIKETHROUGH)
				{
					//closing STRIKETHROUGH environment
					pilePop(&environments);
					translatedString[iTranslated] = '}';
				}
				else
				{
					//opening STRIKETHROUGH environment
					addStrikethroughsToPreamble();
					pilePush(&environments, STRIKETHROUGH);
					translatedStringLength += 5;
					reallocate(&translatedString, translatedStringLength);
               translatedString[iTranslated] = '\0';
					sprintf(translatedString, "%s\\sout{", translatedString);
					iTranslated += 5;
				}
				break;
			case '!':
				if (source[i+1] == '!')
				{
					if (getTop(&environments) == EMPHASIZED)
					{
						//closing EMPHASIZED environment
						pilePop(&environments);
						translatedString[iTranslated] = '}';
						i++;
					}
					else
					{
						//opening EMPHASIZED environment
						pilePush(&environments, EMPHASIZED);
						translatedStringLength += 4;
						reallocate(&translatedString, translatedStringLength);
                  translatedString[iTranslated] = '\0';
						sprintf(translatedString, "%s\\emph{", translatedString);
						iTranslated += 5;
						i++;
					}
				}
				else
					translatedString[iTranslated] = '!';
				break;
         case '[':
            if (isTitle && getTop(&environments) != TELETYPE)
            {
               //opening TELETYPE environment
               pilePush(&environments, TELETYPE);
               translatedStringLength += 8;//making room for the closing '}'
               reallocate(&translatedString, translatedStringLength);
               translatedString[iTranslated] = '\0';
               sprintf(translatedString, "%s\\texttt{", translatedString);
               iTranslated += 7;
            }
            else if (isTitle)//already in TELETYPE environment
               translatedString[iTranslated] = '[';
            else
            {
               //opening PLAIN_TEXT environment
               pilePush(&environments, PLAIN_TEXT);
               translatedStringLength += 6;//already putting a space for the closing '?'
               reallocate(&translatedString, translatedStringLength);
               translatedString[iTranslated] = '\0';
               sprintf(translatedString, "%s\\verb?", translatedString);
               iTranslated += 5;
            }
            break;
         case ']':
            if (!isTitle && getTop(&environments) == PLAIN_TEXT)
            {
               //closing PLAIN_TEXT environment
               pilePop(&environments);
               translatedString[iTranslated] = '?';
            }
            else if (isTitle && getTop(&environments) == TELETYPE)
            {
               //closing TELETYPE environment
               pilePop(&environments);
               translatedString[iTranslated] = '}';
            }
            else//not in TELETYPE or PLAIN_TEXT environment
               translatedString[iTranslated] = ']';
            break;
			default:
				translatedString[iTranslated] = source[i];
				break;
		}
	}

	while (environments.top != NULL)
	{
      char endChar;
		switch(pilePop(&environments))//send warnings on stdout
		{
			case BOLD:
				MD_WARNING(currentLineNb, "Missing bold environment closing tag (**)");
            endChar = '}';
				break;
			case ITALIC:
				MD_WARNING(currentLineNb, "Missing italic environment closing tag (*)");
            endChar = '}';
				break;
			case UNDERLINE:
				MD_WARNING(currentLineNb, "Missing underline environment closing tag (_)");
            endChar = '}';
				break;
			case STRIKETHROUGH:
				MD_WARNING(currentLineNb, "Missing strikethrough environment closing tag (~)");
            endChar = '}';
				break;
			case EMPHASIZED:
				MD_WARNING(currentLineNb, "Missing emphasized environment closing tag (!!)");
            endChar = '}';
				break;
         case PLAIN_TEXT:
            MD_WARNING(currentLineNb, "Missing plain text environment closing tag (])");
            endChar = '?';
            break;
			default:
				MD_WARNING(currentLineNb, "Missing unknown environment closing tag");
            endChar = '}';
				break;
		}
		translatedStringLength++;
		reallocate(&translatedString, translatedStringLength);
		translatedString[iTranslated] = endChar;
		iTranslated++;
	}
	translatedString[iTranslated] = '\0';

	*destination = translatedString;

	pileFree(&environments);
}

void translateToFile(FILE* bodyOutputFile, const char* string)
{
   Pile environments;
   environments.top = NULL;

   for (int i=0; string[i]!='\0'; i++)
   {
      if (getTop(&environments) == PLAIN_TEXT)
      {
         if (string[i] == ']')//close PLAIN_TEXT environment
         {
            fputc('?', bodyOutputFile);
            pilePop(&environments);
         }
         else
            fputc(string[i], bodyOutputFile);
      }
      else if (getTop(&environments) == LATEX)
      {
         if (string[i] == '@' && string[i+1] == '@')//close LATEX environment
         {
            i++;
            pilePop(&environments);
         }
         else
            fputc(string[i], bodyOutputFile);
      }
      else
      {
         switch (string[i])
         {
            case '$':
               fputs("\\$", bodyOutputFile);
               break;
            case '{':
               fputs("\\{", bodyOutputFile);
               break;
            case '}':
               fputs("\\}", bodyOutputFile);
               break;
            case '&':
               fputs("\\&", bodyOutputFile);
               break;
            case '-':
               fputs("\\textendash ", bodyOutputFile);
               break;
            case '.':
               if (string[i+1]=='.' && string[i+2]=='.')//TODO check we're not out of the string
               {
                  i += 2;
                  fputs("\\dots ", bodyOutputFile);
               }
               else
                  fputc('.', bodyOutputFile);
               break;
            case '\\':
               i++;
               switch (string[i])
               {
                  case '\\':
                     fputs("\\textbackslash ", bodyOutputFile);
                     break;
                  case '~':
                     fputs("$\\sim$", bodyOutputFile);
                     break;
                  case '%':
                     fputs("\\%%", bodyOutputFile);
                     break;
                  case '*':
                     fputc('*', bodyOutputFile);
                     break;
                  case '_':
                     fputs("\\_", bodyOutputFile);
                     break;
                  case '!':
                     fputc('!', bodyOutputFile);
                     break;
                  case '#':
                     fputs("\\#", bodyOutputFile);
                     break;
                  case '.':
                     fputc('.', bodyOutputFile);
                     break;
                  case '[':
                     fputc('[', bodyOutputFile);
                     break;
                  case ']':
                     fputc(']', bodyOutputFile);
                     break;
                  case '<':
                     fputc('<', bodyOutputFile);
                     break;
                  case '>':
                     fputc('>', bodyOutputFile);
                     break;
                  case '=':
                     fputc('=', bodyOutputFile);
                     break;
                  case '@':
                     fputc('@', bodyOutputFile);
                     break;
                  default:
                     fprintf(bodyOutputFile, "\\textbackslash %c", string[i]);
                     break;
               }
               break;
            case '*':
               if (string[i+1] != '*')//ITALIC
               {
                  if (getTop(&environments) == ITALIC)//close ITALIC environment
                  {
                     fputc('}', bodyOutputFile);
                     pilePop(&environments);
                  }
                  else//open ITALIC environment
                  {
                     fputs("\\textit{", bodyOutputFile);
                     pilePush(&environments, ITALIC);
                  }
               }
               else//BOLD
               {
                  i++;
                  if (getTop(&environments) == BOLD)//close BOLD environment
                  {
                     fputc('}', bodyOutputFile);
                     pilePop(&environments);
                  }
                  else//open BOLD environment
                  {
                     fputs("\\textbf{", bodyOutputFile);
                     pilePush(&environments, BOLD);
                  }
               }
               break;
            case '_':
               if (getTop(&environments) == UNDERLINE)//close UNDERLINE environment
               {
                  fputc('}', bodyOutputFile);
                  pilePop(&environments);
               }
               else//open UNDERLINE environment
               {
                  fputs("\\underline{", bodyOutputFile);
                  pilePush(&environments, UNDERLINE);
               }
               break;
            case '~':
               if (getTop(&environments) == STRIKETHROUGH)//close STRIKETHROUGH environment
               {
                  fputc('}', bodyOutputFile);
                  pilePop(&environments);
               }
               else//open STRIKETHROUGH environment
               {
                  addStrikethroughsToPreamble();
                  fputs("\\sout{", bodyOutputFile);
                  pilePush(&environments, STRIKETHROUGH);
               }
               break;
            case '!':
               if (string[i+1] != '!')
                  fputc('!', bodyOutputFile);
               else if (getTop(&environments) == EMPHASIZED)//close EMPHASIZED environment
               {
                  i++;
                  fputc('}', bodyOutputFile);
                  pilePop(&environments);
               }
               else//open EMPHASIZED environment
               {
                  i++;
                  fputs("\\emph{", bodyOutputFile);
                  pilePush(&environments, EMPHASIZED);
               }
               break;
            case '>':
               if (string[i+1] != '>')
                  fputs("\\textgreater ", bodyOutputFile);
               else if (getTop(&environments) == QUOTE)//close QUOTE environment
               {
                  i++;
                  fputc('}', bodyOutputFile);
                  pilePop(&environments);
               }
               else//open QUOTE environment
               {
                  i += 2;//pass the ">>"
                  while (string[i]==' ' || string[i]=='\t')
                     i++;
                  i--;//balance the i++ of the for

                  fputs("\\textrm{", bodyOutputFile);
                  pilePush(&environments, QUOTE);
               }
               break;
            case '<':
               if (string[i+1]=='h' && string[i+2]=='t' && string[i+3]=='t' && string[i+4]=='p')//if it begins in "<http"
               {
                  addLinksToPreamble();

                  char* url = pickURL(string, i+1);
                  if (url == NULL)
                  {
                     MD_WARNING(currentLineNb, "Couldn't insert hyperlink");
                     while (string[i]!='>' && string[i]!='\0')
                        i++;
                  }
                  else
                  {
                     char* urlLabel = pickURLLabel(string, i+1);
                     char* translatedUrlLabel;
                     if (urlLabel == NULL)//no label
                        fprintf(bodyOutputFile, "\\url{%s}", url);
                     else//label
                     {
                        translateString(urlLabel, &translatedUrlLabel, false);

                        fprintf(bodyOutputFile, "\\href{%s}{%s}", url, translatedUrlLabel);
                        free(urlLabel);
                        free(translatedUrlLabel);
                     }
                     free(url);

                     while (string[i]!='>' && string[i]!='\0')
                        i++;
                  }
               }
               else
                  fputs("\\textless ", bodyOutputFile);
               break;
            case '[':
               //open PLAIN_TEXT environment
               fputs("\\verb?", bodyOutputFile);
               pilePush(&environments, PLAIN_TEXT);
               break;
            case '@':
               if (string[i+1] != '@')
                  fputc('@', bodyOutputFile);
               else if (getTop(&environments) == LATEX)//close LATEX environment //Should never happen
               {
                  i++;
                  pilePop(&environments);
               }
               else//open LATEX environment
               {
                  i++;
                  pilePush(&environments, LATEX);
               }
               break;
            default:
               fputc(string[i], bodyOutputFile);
               break;
         }
      }
   }

   while (getTop(&environments) != NORMAL)
   {
      Environment popped = pilePop(&environments);
      switch (popped)
      {
         case ITALIC:
            MD_WARNING(currentLineNb, "Missing closing tag for italic environment (*).");
            fputc('}', bodyOutputFile);
            break;
         case BOLD:
            MD_WARNING(currentLineNb, "Missing closing tag for bold environment (**).");
            fputc('}', bodyOutputFile);
            break;
         case UNDERLINE:
            MD_WARNING(currentLineNb, "Missing closing tag for underline environment (_).");
            fputc('}', bodyOutputFile);
            break;
         case STRIKETHROUGH:
            MD_WARNING(currentLineNb, "Missing closing tag for strikethroughed environment (~).");
            fputc('}', bodyOutputFile);
            break;
         case EMPHASIZED:
            MD_WARNING(currentLineNb, "Missing closing tag for emphasized environment (!!).");
            fputc('}', bodyOutputFile);
            break;
         case PLAIN_TEXT:
            MD_WARNING(currentLineNb, "Missing closing tag for plain text (]).");
            fputc('?', bodyOutputFile);
            break;
         case QUOTE://QUOTE environment can be unclosed
            fputc('}', bodyOutputFile);
            break;
         case LATEX:
            MD_WARNING(currentLineNb, "Missing closing tag for not interpreted environment (@@).");
            break;
         default:
            MD_ERROR(currentLineNb, "An unknown environment was detected and is missing a closing tag. Output may not be usable.")
            break;
      }
   }

   pileFree(&environments);
}
