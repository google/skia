#include "SkPdfUtils.h"

bool ArrayFromDictionary(const PdfMemDocument* pdfDoc,
                         const PdfDictionary& dict,
                         const char* key,
                         const char* abr,
                         SkPdfArray* data) {return false;}

bool FileSpecFromDictionary(const PdfMemDocument* pdfDoc,
                         const PdfDictionary& dict,
                         const char* key,
                         const char* abr,
                         SkPdfFileSpec* data) {return false;}

bool StreamFromDictionary(const PdfMemDocument* pdfDoc,
                         const PdfDictionary& dict,
                         const char* key,
                         const char* abr,
                         SkPdfStream** data);

bool TreeFromDictionary(const PdfMemDocument* pdfDoc,
                         const PdfDictionary& dict,
                         const char* key,
                         const char* abr,
                         SkPdfTree** data) {return false;}

bool DateFromDictionary(const PdfMemDocument* pdfDoc,
                         const PdfDictionary& dict,
                         const char* key,
                         const char* abr,
                         SkPdfDate* data) {return false;}

bool FunctionFromDictionary(const PdfMemDocument* pdfDoc,
                         const PdfDictionary& dict,
                         const char* key,
                         const char* abr,
                         SkPdfFunction* data) {return false;}
