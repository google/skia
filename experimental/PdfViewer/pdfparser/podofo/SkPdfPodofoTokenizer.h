#ifndef EXPERIMENTAL_PDFVIEWER_PDFPARSER_PODOFO_SKPDFPODOFOTOKENIZER_H_
#define EXPERIMENTAL_PDFVIEWER_PDFPARSER_PODOFO_SKPDFPODOFOTOKENIZER_H_

#include "stddef.h"

class SkPdfObject;
class SkPdfMapper;
class SkPodofoParsedPDF;

namespace PoDoFo {
class PdfMemDocument;
class PdfContentsTokenizer;
}

enum SkPdfTokenType {
    kKeyword_TokenType,
    kObject_TokenType,
    kImageData_TokenType,  // TODO(edisonn): inline images seem to work without it
};

struct PdfToken {
    const char*      fKeyword;
    SkPdfObject*     fObject;
    SkPdfTokenType   fType;

    PdfToken() : fKeyword(NULL), fObject(NULL) {}
};

class SkPdfPodofoTokenizer {
public:
    SkPdfPodofoTokenizer(const SkPodofoParsedPDF* parser, PoDoFo::PdfContentsTokenizer* tokenizer);
    SkPdfPodofoTokenizer(const SkPdfObject* objWithStream);
    SkPdfPodofoTokenizer(const char* buffer, int len);

    virtual ~SkPdfPodofoTokenizer();

    bool readToken(PdfToken* token);
    bool readTokenCore(PdfToken* token);
    void PutBack(PdfToken token);

private:
    const SkPdfMapper* fMapper;
    const PoDoFo::PdfMemDocument* fDoc;
    PoDoFo::PdfContentsTokenizer* fTokenizer;

    char* fUncompressedStream;
    long fUncompressedStreamLength;

    bool fEmpty;
    bool fHasPutBack;
    PdfToken fPutBack;
};

#endif  // EXPERIMENTAL_PDFVIEWER_PDFPARSER_PODOFO_SKPDFPODOFOTOKENIZER_H_
