/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPdfParser_DEFINED
#define SkPdfParser_DEFINED

#include "SkPdfBasics.h"
#include "SkPdfNativeTokenizer.h"

extern "C" PdfContext* gPdfContext;
extern "C" SkBitmap* gDumpBitmap;
extern "C" SkCanvas* gDumpCanvas;

// TODO(edisonn): Document PdfTokenLooper and subclasses.
class PdfTokenLooper {
protected:
    PdfTokenLooper* fParent;
    SkPdfNativeTokenizer* fTokenizer;
    PdfContext* fPdfContext;
    SkCanvas* fCanvas;

public:
    PdfTokenLooper(PdfTokenLooper* parent,
                   SkPdfNativeTokenizer* tokenizer,
                   PdfContext* pdfContext,
                   SkCanvas* canvas)
        : fParent(parent), fTokenizer(tokenizer), fPdfContext(pdfContext), fCanvas(canvas) {}

    virtual ~PdfTokenLooper() {}

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
                  SkPdfNativeTokenizer* tokenizer,
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

// TODO(edisonn): move in another file
class SkPdfViewer : public SkRefCnt {
public:

  bool load(const SkString inputFileName, SkPicture* out);
  bool write(void*) const { return false; }
};

void reportPdfRenderStats();

#endif  // SkPdfParser_DEFINED
