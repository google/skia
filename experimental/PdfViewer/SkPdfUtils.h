#ifndef __DEFINED__SkPdfUtils
#define __DEFINED__SkPdfUtils

#include "SkPdfBasics.h"

class SkPdfArray;

SkMatrix SkMatrixFromPdfArray(SkPdfArray* pdfArray);

PdfResult doType3Char(PdfContext* pdfContext, SkCanvas* canvas, const SkPdfObject* skobj, SkRect bBox, SkMatrix matrix, double textSize);

#endif   // __DEFINED__SkPdfUtils
