#ifndef _TEX_TO_PDF_
#define _TEX_TO_PDF_

#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "filename.h"

#define PDFLATEX_COMMAND   "pdflatex --interaction=nonstopmode "

STATUS compileTexToPdf(const char* outputFileName);

#endif //_TEX_TO_PDF_
