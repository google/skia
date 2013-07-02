#include "SkPdfPodofoTokenizer.h"

#include "SkTypes.h"
#include "SkPdfStream_autogen.h"
#include "SkPdfMapper_autogen.h"

#include "podofo.h"

// maps to a null doc, if the code asks for it, we should err/crash.
SkPdfMapper gNullMapper(NULL);

SkPdfPodofoTokenizer::SkPdfPodofoTokenizer(const SkPodofoParsedPDF* parser, PoDoFo::PdfContentsTokenizer* tokenizer)
    : fMapper(parser->mapper()), fDoc(parser->podofo()), fTokenizer(tokenizer), fUncompressedStream(NULL), fUncompressedStreamLength(0), fEmpty(false), fHasPutBack(false) {}

SkPdfPodofoTokenizer::SkPdfPodofoTokenizer(const SkPdfObject* objWithStream) : fMapper(&gNullMapper), fDoc(NULL), fTokenizer(NULL), fUncompressedStream(NULL), fUncompressedStreamLength(0), fEmpty(false), fHasPutBack(false) {
    fUncompressedStream = NULL;
    fUncompressedStreamLength = 0;

    fDoc = NULL;

    SkPdfStream* stream = NULL;
    if (objWithStream &&
            objWithStream->doc()->mapper()->mapStream(objWithStream, &stream) &&
            stream->GetFilteredCopy(&fUncompressedStream, &fUncompressedStreamLength) &&
            fUncompressedStream != NULL &&
            fUncompressedStreamLength != 0) {
        fTokenizer = new PoDoFo::PdfContentsTokenizer(fUncompressedStream, fUncompressedStreamLength);
        fDoc = objWithStream->doc()->podofo();
    } else {
        fEmpty = true;
    }
}

SkPdfPodofoTokenizer::SkPdfPodofoTokenizer(const char* buffer, int len) : fMapper(&gNullMapper), fDoc(NULL), fTokenizer(NULL), fUncompressedStream(NULL), fUncompressedStreamLength(0), fEmpty(false), fHasPutBack(false) {
    try {
        fTokenizer = new PoDoFo::PdfContentsTokenizer(buffer, len);
    } catch (PoDoFo::PdfError& e) {
        fEmpty = true;
    }
}

SkPdfPodofoTokenizer::~SkPdfPodofoTokenizer() {
    free(fUncompressedStream);
}

bool SkPdfPodofoTokenizer::readTokenCore(PdfToken* token) {
    PoDoFo::PdfVariant var;
    PoDoFo::EPdfContentsType type;

    token->fKeyword = NULL;
    token->fObject = NULL;

    bool ret = fTokenizer->ReadNext(type, token->fKeyword, var);

    if (!ret) return ret;

    switch (type) {
        case PoDoFo::ePdfContentsType_Keyword:
            token->fType = kKeyword_TokenType;
            break;

        case PoDoFo::ePdfContentsType_Variant: {
                token->fType = kObject_TokenType;
                PoDoFo::PdfObject* obj = new PoDoFo::PdfObject(var);
                fMapper->mapObject(obj, &token->fObject);
            }
            break;

        case PoDoFo::ePdfContentsType_ImageData:
            token->fType = kImageData_TokenType;
            // TODO(edisonn): inline images seem to work without it
            break;
    }
#ifdef PDF_TRACE
    std::string str;
    if (token->fObject) {
        token->fObject->podofo()->ToString(str);
    }
    printf("%s %s\n", token->fType == kKeyword_TokenType ? "Keyword" : token->fType == kObject_TokenType ? "Object" : "ImageData", token->fKeyword ? token->fKeyword : str.c_str());
#endif
    return ret;
}

void SkPdfPodofoTokenizer::PutBack(PdfToken token) {
    SkASSERT(!fHasPutBack);
    fHasPutBack = true;
    fPutBack = token;
}

bool SkPdfPodofoTokenizer::readToken(PdfToken* token) {
    if (fHasPutBack) {
        *token = fPutBack;
        fHasPutBack = false;
        return true;
    }

    if (fEmpty) {
        return false;
    }

    return readTokenCore(token);
}
