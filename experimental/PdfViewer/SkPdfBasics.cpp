#include "SkPdfBasics.h"
#include "SkPdfNativeTokenizer.h"

PdfContext::PdfContext(SkNativeParsedPDF* doc)
    : fPdfDoc(doc)
    , fTmpPageAllocator(new SkPdfAllocator()) {
}

PdfContext::~PdfContext() {
    delete fTmpPageAllocator;
}

