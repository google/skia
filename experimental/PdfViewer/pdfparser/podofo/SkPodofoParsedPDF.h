#ifndef EXPERIMENTAL_PDFVIEWER_PDFPARSER_PODOFO_SKPODOFOPARSEDPDF_H_
#define EXPERIMENTAL_PDFVIEWER_PDFPARSER_PODOFO_SKPODOFOPARSEDPDF_H_

#include "SkRect.h"

class SkCanvas;

class SkPdfInteger;
class SkPdfMapper;
class SkPdfNumber;
class SkPdfObject;
class SkPdfResourceDictionary;
class SkPdfStream;
class SkPdfString;

class SkPdfPodofoTokenizer;

namespace PoDoFo {
class PdfMemDocument;
class PdfObject;
}

class SkPodofoParsedPDF {
public:
    SkPodofoParsedPDF(const char* path);
    virtual ~SkPodofoParsedPDF();

    virtual int pages() const;
    virtual double width(int page) const;
    virtual double height(int page) const;
    const SkPdfResourceDictionary* pageResources(int page) const;
    virtual SkRect MediaBox(int n) const;
    virtual SkPdfPodofoTokenizer* tokenizerOfPage(int n) const;

    virtual SkPdfPodofoTokenizer* tokenizerOfStream(const SkPdfStream* stream) const;
    virtual SkPdfPodofoTokenizer* tokenizerOfBuffer(char* buffer, size_t len) const;

    virtual size_t objects() const;
    virtual const SkPdfObject* object(int i) const;

    PoDoFo::PdfMemDocument* podofo() const {return fDoc;}

    const SkPdfMapper* mapper() const;

    SkPdfNumber* createNumber(double number) const;
    SkPdfInteger* createInteger(int value) const;
    SkPdfString* createString(char* sz, size_t len) const;

    void drawPage(int page, SkCanvas* canvas) const;

private:
    SkPdfObject* make(PoDoFo::PdfObject* obj) const;
    const SkPdfObject* make(const PoDoFo::PdfObject* obj) const;

    PoDoFo::PdfMemDocument* fDoc;
    SkPdfMapper* fMapper;
};

#endif  // EXPERIMENTAL_PDFVIEWER_PDFPARSER_PODOFO_SKPODOFOPARSEDPDF_H_
