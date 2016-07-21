#ifndef _TEX_TO_PDF_
#define _TEX_TO_PDF_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "error.h"
#include "files.h"

#define PDFLATEX_COMMAND            "pdflatex --interaction=nonstopmode "
#define PDFLATEX_COMMAND_REDIRECT   "pdflatex --interaction=nonstopmode %s > /dev/null"

STATUS compileTexToPdf(const char* outputFileName, bool displayPdflatexStdout, bool keepTmpFiles);
void deleteTemporaryFiles(const char* outputFileName);

#endif //_TEX_TO_PDF_
