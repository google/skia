#ifndef EXPERIMENTAL_PDFVIEWER_PDFPARSER_PODOFO_SKPODOFOUTILS_H_
#define EXPERIMENTAL_PDFVIEWER_PDFPARSER_PODOFO_SKPODOFOUTILS_H_

#include <string>
#include "SkPdfNYI.h"

class SkMatrix;
class SkRect;

namespace PoDoFo {
class PdfDictionary;
class PdfObject;
}

class SkPodofoParsedPDF;

const PoDoFo::PdfObject* resolveReferenceObject(const SkPodofoParsedPDF* pdfDoc,
                                  const PoDoFo::PdfObject* obj,
                                  bool resolveOneElementArrays = false);

bool LongFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        long* data);

bool DoubleFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                          const PoDoFo::PdfDictionary& dict,
                          const char* key,
                          const char* abr,
                          double* data);

bool BoolFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        bool* data);

bool NameFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        std::string* data);

bool StringFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                          const PoDoFo::PdfDictionary& dict,
                          const char* key,
                          const char* abr,
                          std::string* data);
/*
class SkPoDoFo::PdfDictionary;
bool DictionaryFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                              const PoDoFo::PdfDictionary& dict,
                              const char* key,
                              const char* abr,
                              SkPoDoFo::PdfDictionary** data);
*/

bool skpdfmap(const SkPodofoParsedPDF& podofoDoc, const PoDoFo::PdfObject& podofoObj, PoDoFo::PdfObject** out);

bool ObjectFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        PoDoFo::PdfObject** data);


class SkPdfArray;
class SkPdfStream;

bool ArrayFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfArray** data);

bool SkMatrixFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkMatrix** data);

bool FileSpecFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfFileSpec* data);


bool StreamFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfStream** data);

bool TreeFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfTree** data);

bool DateFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfDate* data);

bool SkRectFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkRect** data);

bool FunctionFromDictionary(const SkPodofoParsedPDF* pdfDoc,
                        const PoDoFo::PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfFunction* data);

SkMatrix SkMatrixFromPdfMatrix(double array[6]);

#endif  // EXPERIMENTAL_PDFVIEWER_PDFPARSER_PODOFO_SKPODOFOUTILS_H_
