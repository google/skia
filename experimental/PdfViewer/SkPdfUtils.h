#ifndef __DEFINED__SkPdfUtils
#define __DEFINED__SkPdfUtils

#include "podofo.h"
using namespace PoDoFo;

#include "SkPdfBasics.h"

const PdfObject* resolveReferenceObject(const PdfMemDocument* pdfDoc,
                                  const PdfObject* obj,
                                  bool resolveOneElementArrays = false);

bool LongFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        long* data);

bool DoubleFromDictionary(const PdfMemDocument* pdfDoc,
                          const PdfDictionary& dict,
                          const char* key,
                          const char* abr,
                          double* data);

bool BoolFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        bool* data);

bool NameFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        std::string* data);

bool StringFromDictionary(const PdfMemDocument* pdfDoc,
                          const PdfDictionary& dict,
                          const char* key,
                          const char* abr,
                          std::string* data);
/*
class SkPdfDictionary;
bool DictionaryFromDictionary(const PdfMemDocument* pdfDoc,
                              const PdfDictionary& dict,
                              const char* key,
                              const char* abr,
                              SkPdfDictionary** data);
*/

bool skpdfmap(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out);


class SkPdfObject;
bool ObjectFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfObject** data);


struct SkPdfFileSpec {};
class SkPdfArray;
class SkPdfStream;
struct SkPdfDate {};
struct SkPdfTree {};
struct SkPdfFunction {};

bool ArrayFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfArray** data);

bool SkMatrixFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkMatrix** data);

bool FileSpecFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfFileSpec* data);


bool StreamFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfStream** data);

bool TreeFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfTree** data);

bool DateFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfDate* data);

bool SkRectFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkRect** data);

bool FunctionFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfFunction* data);

SkMatrix SkMatrixFromPdfArray(SkPdfArray* pdfArray);

PdfResult doType3Char(PdfContext* pdfContext, SkCanvas* canvas, SkPdfObject* skobj, SkRect bBox, SkMatrix matrix, double textSize);

#include "SkPdfPodofoMapper_autogen.h"

#endif   // __DEFINED__SkPdfUtils
