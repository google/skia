/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfTokenLooper_DEFINED
#define SkPdfTokenLooper_DEFINED

#include "SkPdfNativeTokenizer.h"
// For SkPdfResult
#include "SkPdfUtils.h"

class SkCanvas;
class SkPdfContext;

class SkPdfTokenLooper {
protected:
    SkPdfTokenLooper* fParent;
    SkPdfNativeTokenizer* fTokenizer;
    SkPdfContext* fPdfContext;
    SkCanvas* fCanvas;

public:
    SkPdfTokenLooper(SkPdfTokenLooper* parent,
                   SkPdfNativeTokenizer* tokenizer,
                   SkPdfContext* pdfContext,
                   SkCanvas* canvas)
        : fParent(parent), fTokenizer(tokenizer), fPdfContext(pdfContext), fCanvas(canvas) {}

    virtual ~SkPdfTokenLooper() {}

    virtual SkPdfResult consumeToken(PdfToken& token) = 0;
    virtual void loop() = 0;

    void setUp(SkPdfTokenLooper* parent) {
        fParent = parent;
        fTokenizer = parent->fTokenizer;
        fPdfContext = parent->fPdfContext;
        fCanvas = parent->fCanvas;
    }
};

// Calls SkPdfNativeTokenizer::readToken, and also does debugging help.
// TODO(edisonn): Pass SkPdfContext and SkCanvas only with the define for instrumentation.
// FIXME (scroggo): This calls tokenizer->readToken(). The rest of its functionality should
// be moved to a debugging file.
bool readToken(SkPdfNativeTokenizer*, PdfToken*);

#endif // SkPdfTokenLooper_DEFINED
