#include "compiler.h"

FILE* inputFile = NULL;
int currentLineNb = 0;
char nbAlinea = 0;

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
   translateLineByLine(tmpBodyOutputFile);
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
   if (deleteFile(tmpBodyOutputFilePath) != RETURN_SUCCESS)
   {
      free(tmpBodyOutputFilePath);
      return RETURN_FAILURE;
   }
   free(tmpBodyOutputFilePath);

   return RETURN_SUCCESS;
}

void translateLineByLine(FILE* bodyOutputFile)
{
   fputs("\\begin{document}\n", bodyOutputFile);
   fputs("\\maketitle\n\n", bodyOutputFile);

   char* line;
   int firstNonSpaceIndex;
   while ((line = getNextLineFromFile()) != NULL)
   {
      currentLineNb++;
      for (firstNonSpaceIndex=0; line[firstNonSpaceIndex]==' ' || line[firstNonSpaceIndex]=='\t'; firstNonSpaceIndex++)
         ;//remove alinea
      interpretLine(bodyOutputFile, &line[firstNonSpaceIndex]);
      free(line);
   }

   fputs("\\end{document}\n", bodyOutputFile);
}

void interpretLine(FILE* bodyOutputFile, const char* line)
{
   if (line[0] == '#')
   {
      char* partTitle = getTitleOfPart(line);
      char* translatedTitle = NULL;
      translateString(partTitle, &translatedTitle);
      removeUselessSpaces(translatedTitle);

      int commentIndex = getFirstIndexOfComment(line);

      if (line[1] != '#')//title
      {
         addLineToTitle(translatedTitle);
         if (commentIndex >= 0)
            addCommentToTitle(&line[commentIndex]);

         nbAlinea = 1;
      }
      else if (line[2] != '#')//part
      {
         if (commentIndex >= 0)
            fprintf(bodyOutputFile, "\t\\part{%s}%s\n\n", translatedTitle, &line[commentIndex]);
         else
            fprintf(bodyOutputFile, "\t\\part{%s}\n\n", translatedTitle);

         nbAlinea = 2;
      }
      else if (line[3] != '#')//section
      {
         if (commentIndex >= 0)
            fprintf(bodyOutputFile, "\t\t\\section{%s}%s\n\n", translatedTitle, &line[commentIndex]);
         else
            fprintf(bodyOutputFile, "\t\t\\section{%s}\n\n", translatedTitle);

         nbAlinea = 3;
      }
      else if (line[4] != '#')//subsection
      {
         if (commentIndex >= 0)
            fprintf(bodyOutputFile, "\t\t\t\\subsection{%s}%s\n\n", translatedTitle, &line[commentIndex]);
         else
            fprintf(bodyOutputFile, "\t\t\t\\subsection{%s}\n\n", translatedTitle);

         nbAlinea = 4;
      }
      else if (line[5] != '#')//subsubsection
      {
         if (commentIndex >= 0)
            fprintf(bodyOutputFile, "\t\t\t\t\\subsubsection{%s}%s\n\n", translatedTitle, &line[commentIndex]);
         else
            fprintf(bodyOutputFile, "\t\t\t\t\\subsubsection{%s}\n\n", translatedTitle);

         nbAlinea = 5;
      }
      else if (line[6] != '#')//paragraph
      {
         if (commentIndex >= 0)
            fprintf(bodyOutputFile, "\t\t\t\t\t\\paragraph{%s}%s\n", translatedTitle, &line[commentIndex]);
         else
            fprintf(bodyOutputFile, "\t\t\t\t\t\\paragraph{%s}\n", translatedTitle);

         nbAlinea = 6;
      }
      else//subparagraph
      {
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
      int beginningLineNb = currentLineNb;

      char* nextLine = getNextLineFromFile();
      currentLineNb++;

      fputs("\\begin{verbatim}\n", bodyOutputFile);

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
         currentLineNb++;
         if (nextLine == NULL)
            break;
      }
      if (nextLine != NULL)
         free(nextLine);
      else
         MD_WARNING(beginningLineNb, "Missing closing tag for multiline plain text (])");

      fputs("\\end{verbatim}\n", bodyOutputFile);
   }
   else if (isImageLine(line))
   {
      addImagesToPreamble();

      writeAlinea(bodyOutputFile);
      fputs("\\begin{figure}[h!]\n", bodyOutputFile);
      nbAlinea++;

      writeAlinea(bodyOutputFile);
      fputs("\\centering\n", bodyOutputFile);

      char* imageFileName = pickImageFileName(line);
      //TODO check if the image exists
      if (imageFileName != NULL)
      {
         writeAlinea(bodyOutputFile);
         fprintf(bodyOutputFile, "\\includegraphics{%s}\n", imageFileName);
         free(imageFileName);

         char* label = pickImageLabel(line);
         if (label != NULL)
         {
            writeAlinea(bodyOutputFile);
            fprintf(bodyOutputFile, "\\caption{%s}\n", label);
            free(label);

            //TODO sizes
         }
      }
      else
         MD_WARNING(currentLineNb, "Couldn't include image");

      nbAlinea--;
      writeAlinea(bodyOutputFile);
      fputs("\\end{figure}\n\n", bodyOutputFile);
   }
   else if (isItemizeLine(line))
   {
      addEnumToPreamble();
      writeAlinea(bodyOutputFile);
      fputs("\\begin{itemize}[label=$\\bullet$]\n", bodyOutputFile);
      nbAlinea++;

      writeAlinea(bodyOutputFile);
      fputs("\\item\n", bodyOutputFile);

      char* item = pickItemFromItemize(line);
      writeAlinea(bodyOutputFile);
      fprintf(bodyOutputFile, "\t%s\n", item);
      free(item);

      char* nextLine = getNextLineFromFile();
      currentLineNb++;
      while (nextLine != NULL && isItemizeLine(nextLine))
      {
         writeAlinea(bodyOutputFile);
         fputs("\\item\n", bodyOutputFile);

         item = pickItemFromItemize(nextLine);
         writeAlinea(bodyOutputFile);
         fprintf(bodyOutputFile, "\t%s\n", item);
         free(item);

         //printf("cursor : %ld\n", ftell(bodyOutputFile));

         free(nextLine);
         nextLine = getNextLineFromFile();
         currentLineNb++;
      }

      nbAlinea--;
      writeAlinea(bodyOutputFile);
      fputs("\\end{itemize}\n\n", bodyOutputFile);

      if (nextLine != NULL)
      {
         //put cursor back to beginning of line
         //printf("size : %d\t->cursor : %ld\n", strlen(nextLine)+1, ftell(bodyOutputFile));
         fseek(inputFile, -(strlen(nextLine)+1), SEEK_CUR);
         currentLineNb--;
         //printf("cursor : %ld\n\n", ftell(bodyOutputFile));
         free(nextLine);
      }
   }
   else
   {
      writeAlinea(bodyOutputFile);
      translateToFile(bodyOutputFile, line);
      fputc('\n', bodyOutputFile);
      fputc('\n', bodyOutputFile);//to have a new line in the pdf document
   }
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

STATUS deleteFile(const char* filePath)
{
   if (unlink(filePath) != 0)
   {
      char msg[256] = "Couldn't delete temporary file";
      snprintf(msg, 256, "%s %s", msg, filePath);
      ERROR_MSG("deleteFile", msg);
      return RETURN_FAILURE;
   }
   return RETURN_SUCCESS;
}

char* getTitleOfPart(const char* line)
{
   if (line[0] != '#')
   {
      WARNING_FUNC("getTitleOfPart", "Tried to get a title from a non-title line")
      return NULL;
   }

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
   if (string[0] == ' ' || string[0] == '\t')
   {
      int i;
      for (i=0; string[i]==' '||string[i]=='\t'; i++)
         ;
      string = &string[i];
   }

   int lastCharIndex = strlen(string)-1;
   while (string[lastCharIndex]==' ' || string[lastCharIndex]=='\t')
   {
      string[lastCharIndex] = '\0';
      lastCharIndex = strlen(string)-1;
   }
}

bool isMultilinePlainTextOpeningTag(const char* line)
{
   int firstUsefulPartIndex;
   for (firstUsefulPartIndex=0; line[firstUsefulPartIndex]==' ' || line[firstUsefulPartIndex]=='\t'; firstUsefulPartIndex++)
      ;

   if (line[firstUsefulPartIndex]!='[')
      return false;

   int i;
   for (i=firstUsefulPartIndex+1; line[i]==' ' || line[i]=='\t'; i++)
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
   int firstUsefulPartIndex;
   for (firstUsefulPartIndex=0; line[firstUsefulPartIndex]==' '||line[firstUsefulPartIndex]=='\t'; firstUsefulPartIndex++)
      ;

   //if line begins in <img
   return (line[firstUsefulPartIndex]=='<' && line[firstUsefulPartIndex+1]=='i' && line[firstUsefulPartIndex+2]=='m' && line[firstUsefulPartIndex+3]=='g');
}

void writeAlinea(FILE* file)
{
   for (int i=0; i<nbAlinea; i++)
      fputc('\t', file);
}

char* pickImageFileName(const char* line)
{
   char* tmp = malloc(strlen(line)*sizeof(char));

   int firstUsefulPartIndex;
   for (firstUsefulPartIndex=0; line[firstUsefulPartIndex]==' ' || line[firstUsefulPartIndex]=='\t'; firstUsefulPartIndex++)
      ;

   int firstNameIndex;
   for (firstNameIndex=firstUsefulPartIndex+4; line[firstNameIndex]==' ' || line[firstNameIndex]=='\t'; firstNameIndex++)
      ;

   if (firstNameIndex == firstUsefulPartIndex+4)
   {
      MD_ERROR(currentLineNb, "This line isn't a valid line characterizing the inclusion of an image\nWrite '\\<' instead of '<' if it isn't supposed to include an image");
      free(tmp);
      return NULL;
   }

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
   char* tmp = malloc(strlen(line)*sizeof(char));

   int firstUsefulPartIndex;
   for (firstUsefulPartIndex=0; line[firstUsefulPartIndex]==' ' || line[firstUsefulPartIndex]=='\t'; firstUsefulPartIndex++)
      ;

   int firstNameIndex;
   for (firstNameIndex=firstUsefulPartIndex+4; line[firstNameIndex]==' ' || line[firstNameIndex]=='\t'; firstNameIndex++)
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

   int iDest;
   for (int i=firstLabelIndex; line[i]!='>' && line[i]!=' ' && line[i]!='\t' && line[i]!='\0'; i++, iDest++)
      tmp[iDest] = line[i];
   tmp[iDest] = '\0';

   char* label = malloc( (strlen(tmp)+1)*sizeof(char) );
   strcpy(label, tmp);
   free(tmp);

   return label;
}

char* pickURL(const char* line, int firstURLIndex)
{
   if (firstURLIndex <= 0)
   {
      WARNING_FUNC("pickURL", "Wrong index specified");
      return NULL;
   }

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

char* pickURLLabel(const char* line, int firstURLIndex)
{
   if (firstURLIndex <= 0)
   {
      WARNING_FUNC("pickURLLabel", "Wrong index specified");
      return NULL;
   }

   char* tmp = malloc(strlen(line)*sizeof(char));

   int firstSpaceIndex;
   for (firstSpaceIndex=firstURLIndex; line[firstSpaceIndex]!='>' && line[firstSpaceIndex]!=' ' && line[firstSpaceIndex]!='\t' && line[firstSpaceIndex]!='\0'; firstSpaceIndex++)
      ;

   if (line[firstSpaceIndex]=='\0')
   {
      MD_ERROR(currentLineNb, "Missing closing tag for hyperlink (>)");
      free(tmp);
      return NULL;
   }
   if (line[firstSpaceIndex]=='>')
   {
      free(tmp);
      return NULL;
   }
   //line[firstSpaceIndex]==' ' || line[firstSpaceIndex]=='\t'
   int firstLabelIndex;
   for (firstLabelIndex=firstSpaceIndex; line[firstLabelIndex]==' ' || line[firstLabelIndex]=='\t'; firstLabelIndex++)
      ;

   int i, iDest = 0;
   for (i=firstLabelIndex; line[i]!='>' && line[i]!=' ' && line[i]!='\t' && line[i]!='\0'; i++, iDest++)
      tmp[iDest] = line[i];
   tmp[iDest] = '\0';

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
      MD_ERROR(currentLineNb, "There was a problem reading this enumeration, the line must begin with '+'");
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

void translateString(const char* source, char** destination)
{
	int translatedStringLength = strlen(source)+1;
	char* translatedString = malloc(translatedStringLength*sizeof(char));
	translatedString[0] = '\0';

	Pile environments;
	environments.top = NULL;

	int i, iTranslated;
	for (i=0, iTranslated = 0; source[i] != '\0'; i++, iTranslated++)
	{
		switch (source[i])
		{
			case '\\':
				switch (source[i+1])
				{
					case '\\':
						translatedStringLength += 13;
						reallocate(&translatedString, translatedStringLength);
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
						sprintf(translatedString, "%s\\emph{", translatedString);
						iTranslated += 5;
						i++;
					}
				}
				else
					translatedString[iTranslated] = '!';
				break;
			default:
				translatedString[iTranslated] = source[i];
				break;
		}
	}

	while (environments.top != NULL)
	{
		switch(pilePop(&environments))//send warnings on stdout
		{
			case BOLD:
				MD_WARNING(currentLineNb, "Missing bold environment closing tag (**)");
				break;
			case ITALIC:
				MD_WARNING(currentLineNb, "Missing italic environment closing tag (*)");
				break;
			case UNDERLINE:
				MD_WARNING(currentLineNb, "Missing underline environment closing tag (_)");
				break;
			case STRIKETHROUGH:
				MD_WARNING(currentLineNb, "Missing strikethrough environment closing tag (~)");
				break;
			case EMPHASIZED:
				MD_WARNING(currentLineNb, "Missing emphasized environment closing tag (!!)");
				break;
			default:
				MD_WARNING(currentLineNb, "Missing unknown environment closing tag");
				break;
		}
		translatedStringLength++;
		reallocate(&translatedString, translatedStringLength);
		translatedString[iTranslated] = '}';
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
         if (string[i] == ']')//closr PLAIN_TEXT environment
         {
            fputc('?', bodyOutputFile);
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
                  i++;
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
                     if (urlLabel == NULL)//no label
                        fprintf(bodyOutputFile, "\\url{%s}", url);
                     else//label
                     {
                        fprintf(bodyOutputFile, "\\href{%s}{%s}", url, urlLabel);
                        free(urlLabel);
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
            MD_WARNING(currentLineNb, "Missing closing tag for italic environment (*)");
            fputc('}', bodyOutputFile);
            break;
         case BOLD:
            MD_WARNING(currentLineNb, "Missing closing tag for bold environment (**)");
            fputc('}', bodyOutputFile);
            break;
         case UNDERLINE:
            MD_WARNING(currentLineNb, "Missing closing tag for underline environment (_)");
            fputc('}', bodyOutputFile);
            break;
         case STRIKETHROUGH:
            MD_WARNING(currentLineNb, "Missing closing tag for strikethroughed environment (~)");
            fputc('}', bodyOutputFile);
            break;
         case EMPHASIZED:
            MD_WARNING(currentLineNb, "Missing closing tag for emphasized environment (!!)");
            fputc('}', bodyOutputFile);
            break;
         case PLAIN_TEXT:
            MD_WARNING(currentLineNb, "Missing closing tag for plain text (])");
            fputc('?', bodyOutputFile);
            break;
         default://QUOTE environment can be unclosed
            break;
      }
   }

   pileFree(&environments);
}
