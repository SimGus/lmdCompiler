#ifndef _TEX_TO_PDF_
#define _TEX_TO_PDF_

#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "filename.h"

#define PDFLATEX_COMMAND   "pdflatex --interaction=nonstopmode --jobname="

STATUS compileTexToPdf(const char* texFileName, const char* pdfFileName);

#endif //_TEX_TO_PDF_
