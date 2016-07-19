#ifndef _TEX_TO_PDF_
#define _TEX_TO_PDF_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "error.h"
#include "filename.h"

#define PDFLATEX_COMMAND            "pdflatex --interaction=nonstopmode "
#define PDFLATEX_COMMAND_REDIRECT   "pdflatex --interaction=nonstopmode %s > /dev/null"

STATUS compileTexToPdf(const char* outputFileName, bool displayPdflatexStdout);

#endif //_TEX_TO_PDF_
