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
    PdfMemDocument* fDoc;
    PdfContentsTokenizer* fTokenizer;

    char* fUncompressedStream;
    pdf_long fUncompressedStreamLength;

    bool fEmpty;
    bool fHasPutBack;
    PdfToken fPutBack;

public:
    SkPdfTokenizer(PdfMemDocument* doc = NULL, PdfContentsTokenizer* tokenizer = NULL) : fDoc(doc), fTokenizer(tokenizer), fUncompressedStream(NULL), fUncompressedStreamLength(0),  fEmpty(false), fHasPutBack(false) {}
    SkPdfTokenizer(const SkPdfObject* objWithStream) : fDoc(NULL), fTokenizer(NULL), fEmpty(false), fHasPutBack(false) {
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

    SkPdfTokenizer(const char* buffer, int len) : fDoc(NULL), fTokenizer(NULL), fUncompressedStream(NULL), fUncompressedStreamLength(0), fEmpty(false), fHasPutBack(false) {
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
                    mapObject(*fDoc, *obj, &token->fObject);
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

extern "C" PdfContext* gPdfContext;
extern "C" SkBitmap* gDumpBitmap;
extern "C" SkCanvas* gDumpCanvas;

// TODO(edisonn): move in trace util.
#ifdef PDF_TRACE
static void SkTraceMatrix(const SkMatrix& matrix, const char* sz = "") {
    printf("SkMatrix %s ", sz);
    for (int i = 0 ; i < 9 ; i++) {
        printf("%f ", SkScalarToDouble(matrix.get(i)));
    }
    printf("\n");
}

static void SkTraceRect(const SkRect& rect, const char* sz = "") {
    printf("SkRect %s ", sz);
    printf("x = %f ", SkScalarToDouble(rect.x()));
    printf("y = %f ", SkScalarToDouble(rect.y()));
    printf("w = %f ", SkScalarToDouble(rect.width()));
    printf("h = %f ", SkScalarToDouble(rect.height()));
    printf("\n");
}

#else
#define SkTraceMatrix(a,b)
#define SkTraceRect(a,b)
#endif

// TODO(edisonn): Document PdfTokenLooper and subclasses.
class PdfTokenLooper {
protected:
    PdfTokenLooper* fParent;
    SkPdfTokenizer* fTokenizer;
    PdfContext* fPdfContext;
    SkCanvas* fCanvas;

public:
    PdfTokenLooper(PdfTokenLooper* parent,
                   SkPdfTokenizer* tokenizer,
                   PdfContext* pdfContext,
                   SkCanvas* canvas)
        : fParent(parent), fTokenizer(tokenizer), fPdfContext(pdfContext), fCanvas(canvas) {}

    virtual PdfResult consumeToken(PdfToken& token) = 0;
    virtual void loop() = 0;

    void setUp(PdfTokenLooper* parent) {
        fParent = parent;
        fTokenizer = parent->fTokenizer;
        fPdfContext = parent->fPdfContext;
        fCanvas = parent->fCanvas;
    }
};

class PdfMainLooper : public PdfTokenLooper {
public:
    PdfMainLooper(PdfTokenLooper* parent,
                  SkPdfTokenizer* tokenizer,
                  PdfContext* pdfContext,
                  SkCanvas* canvas)
        : PdfTokenLooper(parent, tokenizer, pdfContext, canvas) {}

    virtual PdfResult consumeToken(PdfToken& token);
    virtual void loop();
};

class PdfInlineImageLooper : public PdfTokenLooper {
public:
    PdfInlineImageLooper()
        : PdfTokenLooper(NULL, NULL, NULL, NULL) {}

    virtual PdfResult consumeToken(PdfToken& token);
    virtual void loop();
    PdfResult done();
};

class PdfCompatibilitySectionLooper : public PdfTokenLooper {
public:
    PdfCompatibilitySectionLooper()
        : PdfTokenLooper(NULL, NULL, NULL, NULL) {}

    virtual PdfResult consumeToken(PdfToken& token);
    virtual void loop();
};

class SkPdfDoc {
    PdfMemDocument fDoc;
public:

    PdfMemDocument& podofo() {return fDoc;}

    SkPdfDoc(const char* path) : fDoc(path) {}

    int pages() {
        return fDoc.GetPageCount();
    }

    double width(int n) {
        PdfRect rect = fDoc.GetPage(n)->GetMediaBox();
        return rect.GetWidth() + rect.GetLeft();
    }

    double height(int n) {
        PdfRect rect = fDoc.GetPage(n)->GetMediaBox();
        return rect.GetHeight() + rect.GetBottom();
    }

    // Can return NULL
    SkPdfPageObjectDictionary* page(int n) {
        SkPdfPageObjectDictionary* page = NULL;
        mapPageObjectDictionary(fDoc, *fDoc.GetPage(n)->GetObject(), &page);
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

    void drawPage(int n, SkCanvas* canvas) {
        SkPdfPageObjectDictionary* pg = page(n);
        SkPdfTokenizer* tokenizer = tokenizerOfPage(n);

        PdfContext pdfContext(this);
        pdfContext.fOriginalMatrix = SkMatrix::I();
        pdfContext.fGraphicsState.fResources = NULL;
        mapResourceDictionary(*pg->Resources(), &pdfContext.fGraphicsState.fResources);

        gPdfContext = &pdfContext;
        gDumpCanvas = canvas;

        // TODO(edisonn): get matrix stuff right.
        // TODO(edisonn): add DPI/scale/zoom.
        SkScalar z = SkIntToScalar(0);
        SkRect rect = MediaBox(n);
        SkScalar w = rect.width();
        SkScalar h = rect.height();

        SkPoint pdfSpace[4] = {SkPoint::Make(z, z), SkPoint::Make(w, z), SkPoint::Make(w, h), SkPoint::Make(z, h)};
//                SkPoint skiaSpace[4] = {SkPoint::Make(z, h), SkPoint::Make(w, h), SkPoint::Make(w, z), SkPoint::Make(z, z)};

        // TODO(edisonn): add flag for this app to create sourunding buffer zone
        // TODO(edisonn): add flagg for no clipping.
        // Use larger image to make sure we do not draw anything outside of page
        // could be used in tests.

#ifdef PDF_DEBUG_3X
        SkPoint skiaSpace[4] = {SkPoint::Make(w+z, h+h), SkPoint::Make(w+w, h+h), SkPoint::Make(w+w, h+z), SkPoint::Make(w+z, h+z)};
#else
        SkPoint skiaSpace[4] = {SkPoint::Make(z, h), SkPoint::Make(w, h), SkPoint::Make(w, z), SkPoint::Make(z, z)};
#endif
        //SkPoint pdfSpace[2] = {SkPoint::Make(z, z), SkPoint::Make(w, h)};
        //SkPoint skiaSpace[2] = {SkPoint::Make(w, z), SkPoint::Make(z, h)};

        //SkPoint pdfSpace[2] = {SkPoint::Make(z, z), SkPoint::Make(z, h)};
        //SkPoint skiaSpace[2] = {SkPoint::Make(z, h), SkPoint::Make(z, z)};

        //SkPoint pdfSpace[3] = {SkPoint::Make(z, z), SkPoint::Make(z, h), SkPoint::Make(w, h)};
        //SkPoint skiaSpace[3] = {SkPoint::Make(z, h), SkPoint::Make(z, z), SkPoint::Make(w, 0)};

        SkAssertResult(pdfContext.fOriginalMatrix.setPolyToPoly(pdfSpace, skiaSpace, 4));
        SkTraceMatrix(pdfContext.fOriginalMatrix, "Original matrix");


        pdfContext.fGraphicsState.fMatrix = pdfContext.fOriginalMatrix;
        pdfContext.fGraphicsState.fMatrixTm = pdfContext.fGraphicsState.fMatrix;
        pdfContext.fGraphicsState.fMatrixTlm = pdfContext.fGraphicsState.fMatrix;

        canvas->setMatrix(pdfContext.fOriginalMatrix);

#ifndef PDF_DEBUG_NO_PAGE_CLIPING
        canvas->clipRect(SkRect::MakeXYWH(z, z, w, h), SkRegion::kIntersect_Op, true);
#endif

// erase with red before?
//        SkPaint paint;
//        paint.setColor(SK_ColorRED);
//        canvas->drawRect(rect, paint);

        PdfMainLooper looper(NULL, tokenizer, &pdfContext, canvas);
        looper.loop();

        delete tokenizer;


        canvas->flush();
    }

    SkPdfTokenizer* tokenizerOfPage(int n) {
        PdfContentsTokenizer* t = new PdfContentsTokenizer(fDoc.GetPage(n));
        return new SkPdfTokenizer(&fDoc, t);
    }
};

// TODO(edisonn): move in another file
class SkPdfViewer : public SkRefCnt {
public:

  bool load(const SkString inputFileName, SkPicture* out);
  bool write(void*) const { return false; }
};

void reportPdfRenderStats();

#endif  // SkPdfParser_DEFINED
