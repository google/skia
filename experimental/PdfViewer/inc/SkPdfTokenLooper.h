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

/**
 *  An object which reads tokens from a tokenizer and draws it to an SkCanvas.
 *  FIXME (scroggo): Can this be an interface? See http://goo.gl/AXQtkH
 */
class SkPdfTokenLooper {
public:
    /**
     *  Create a looper with no parent.
     *  @param tokenizer SkPdfNativeTokenizer for reading tokens.
     *  @param pdfContext Context for drawing state.
     *  @param canvas Target SkCanvas for drawing.
     */
    SkPdfTokenLooper(SkPdfNativeTokenizer* tokenizer,
                     SkPdfContext* pdfContext,
                     SkCanvas* canvas)
        : fParent(NULL)
        , fTokenizer(tokenizer)
        , fPdfContext(pdfContext)
        , fCanvas(canvas) {}

    /**
     *  Create a looper as a child of parent. It will share the
     *  SkPdfContext, SkPdfTokenizer, and SkCanvas with its parent.
     */
    explicit SkPdfTokenLooper(SkPdfTokenLooper* parent)
        : fParent(parent)
        , fTokenizer(parent->fTokenizer)
        , fPdfContext(parent->fPdfContext)
        , fCanvas(parent->fCanvas) {}

    virtual ~SkPdfTokenLooper() {}

    /**
     *  Consume a token, and draw to fCanvas as directed.
     */
    virtual SkPdfResult consumeToken(PdfToken& token) = 0;

    /**
     *  Consume all the tokens this looper can handle.
     */
    virtual void loop() = 0;

protected:
    // All are unowned pointers.
    SkPdfTokenLooper*       fParent;
    SkPdfNativeTokenizer*   fTokenizer;
    SkPdfContext*           fPdfContext;
    SkCanvas*               fCanvas;
};

#endif // SkPdfTokenLooper_DEFINED
