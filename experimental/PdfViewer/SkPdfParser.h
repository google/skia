/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "podofo.h"
#include "SkPdfHeaders_autogen.h"
#include "SkPdfPodofoMapper_autogen.h"

#ifndef SkPdfParser_DEFINED
#define SkPdfParser_DEFINED


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

class SkPdfTokenizer {
    PdfContentsTokenizer* fTokenizer;
    PdfMemDocument* fDoc;

    char* fUncompressedStream;
    pdf_long fUncompressedStreamLength;

    bool fEmpty;
    bool fHasPutBack;
    PdfToken fPutBack;

public:
    SkPdfTokenizer(PdfMemDocument* doc = NULL, PdfContentsTokenizer* tokenizer = NULL) : fDoc(doc), fTokenizer(tokenizer), fEmpty(false), fUncompressedStream(NULL), fUncompressedStreamLength(0), fHasPutBack(false) {}
    SkPdfTokenizer(const SkPdfObject* objWithStream) : fDoc(NULL), fTokenizer(NULL), fHasPutBack(false), fEmpty(false) {
        fUncompressedStream = NULL;
        fUncompressedStreamLength = 0;

        fDoc = NULL;


        try {
            objWithStream->podofo()->GetStream()->GetFilteredCopy(&fUncompressedStream, &fUncompressedStreamLength);
            if (fUncompressedStream != NULL && fUncompressedStreamLength != 0) {
                fTokenizer = new PdfContentsTokenizer(fUncompressedStream, fUncompressedStreamLength);
            } else {
                fEmpty = true;
            }
        } catch (PdfError& e) {
            fEmpty = true;
        }

    }

    SkPdfTokenizer(const char* buffer, int len) : fDoc(NULL), fTokenizer(NULL), fHasPutBack(false), fUncompressedStream(NULL), fUncompressedStreamLength(0), fEmpty(false) {
        try {
            fTokenizer = new PdfContentsTokenizer(buffer, len);
        } catch (PdfError& e) {
            fEmpty = true;
        }
    }

    ~SkPdfTokenizer() {
        free(fUncompressedStream);
    }

    void PutBack(PdfToken token) {
        SkASSERT(!fHasPutBack);
        fHasPutBack = true;
        fPutBack = token;
    }

    bool readToken(PdfToken* token) {
        if (fHasPutBack) {
            *token = fPutBack;
            fHasPutBack = false;
            return true;
        }

        if (fEmpty) {
            return false;
        }

        PdfVariant var;
        EPdfContentsType type;

        token->fKeyword = NULL;
        token->fObject = NULL;

        bool ret = fTokenizer->ReadNext(type, token->fKeyword, var);

        if (!ret) return ret;

        switch (type) {
            case ePdfContentsType_Keyword:
                token->fType = kKeyword_TokenType;
                break;

            case ePdfContentsType_Variant: {
                    token->fType = kObject_TokenType;
                    PdfObject* obj = new PdfObject(var);
                    PodofoMapper::map(*fDoc, *obj, &token->fObject);
                }
                break;

            case ePdfContentsType_ImageData:
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
};

class SkPdfDoc {
    PdfMemDocument fDoc;
public:

    PdfMemDocument& podofo() {return fDoc;}

    SkPdfDoc(const char* path) : fDoc(path) {}

    int pages() {
        return fDoc.GetPageCount();
    }

    // Can return NULL
    SkPdfPageObjectDictionary* page(int n) {
        SkPdfPageObjectDictionary* page = NULL;
        PodofoMapper::map(fDoc, *fDoc.GetPage(n)->GetObject(), &page);
        return page;
    }

    SkRect MediaBox(int n) {
        PdfRect rect = fDoc.GetPage(n)->GetMediaBox();
        SkRect skrect = SkRect::MakeLTRB(SkDoubleToScalar(rect.GetLeft()),
                                         SkDoubleToScalar(rect.GetBottom()),
                                         SkDoubleToScalar(rect.GetLeft() + rect.GetWidth()),
                                         SkDoubleToScalar(rect.GetBottom() + rect.GetHeight()));
        return skrect;
    }

    SkPdfTokenizer* tokenizerOfPage(int n) {
        PdfContentsTokenizer* t = new PdfContentsTokenizer(fDoc.GetPage(n));
        return new SkPdfTokenizer(&fDoc, t);
    }
};

#endif  // SkPdfParser_DEFINED
