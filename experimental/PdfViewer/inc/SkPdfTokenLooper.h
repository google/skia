/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfTokenLooper_DEFINED
#define SkPdfTokenLooper_DEFINED

class SkCanvas;
class SkPdfNativeTokenizer;
class SkPdfContext;

class PdfTokenLooper {
protected:
    PdfTokenLooper* fParent;
    SkPdfNativeTokenizer* fTokenizer;
    SkPdfContext* fPdfContext;
    SkCanvas* fCanvas;

public:
    PdfTokenLooper(PdfTokenLooper* parent,
                   SkPdfNativeTokenizer* tokenizer,
                   SkPdfContext* pdfContext,
                   SkCanvas* canvas)
        : fParent(parent), fTokenizer(tokenizer), fPdfContext(pdfContext), fCanvas(canvas) {}

    virtual ~PdfTokenLooper() {}

    virtual SkPdfResult consumeToken(PdfToken& token) = 0;
    virtual void loop() = 0;

    void setUp(PdfTokenLooper* parent) {
        fParent = parent;
        fTokenizer = parent->fTokenizer;
        fPdfContext = parent->fPdfContext;
        fCanvas = parent->fCanvas;
    }

    SkPdfNativeTokenizer* tokenizer() { return fTokenizer; }
};
#endif // SkPdfTokenLooper_DEFINED
