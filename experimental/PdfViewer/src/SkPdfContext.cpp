/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfContext.h"
#include "SkPdfNativeDoc.h"
#include "SkPdfReporter.h"
#include "SkPdfTokenLooper.h"

///////////////////////////////////////////////////////////////////////////////

class PdfMainLooper : public SkPdfTokenLooper {
public:
    PdfMainLooper(SkPdfTokenLooper* parent,
                  SkPdfNativeTokenizer* tokenizer,
                  SkPdfContext* pdfContext,
                  SkCanvas* canvas)
        : SkPdfTokenLooper(parent, tokenizer, pdfContext, canvas) {}

    virtual SkPdfResult consumeToken(PdfToken& token);
    virtual void loop();
};

///////////////////////////////////////////////////////////////////////////////

SkPdfContext::SkPdfContext(SkPdfNativeDoc* doc)
    : fPdfDoc(doc)
{
    SkASSERT(fPdfDoc != NULL);
}

void SkPdfContext::parseStream(SkPdfNativeObject* stream, SkCanvas* canvas) {
    SkPdfNativeTokenizer* tokenizer = fPdfDoc->tokenizerOfStream(stream, &fTmpPageAllocator);
    if (NULL == tokenizer) {
        // Nothing to parse.
        return;
    }
    PdfMainLooper looper(NULL, tokenizer, this, canvas);
    looper.loop();
    // FIXME (scroggo): Will restructure to put tokenizer on the stack.
    delete tokenizer;
}

///////////////////////////////////////////////////////////////////////////////

// FIXME (scroggo): This probably belongs in a debugging file.
// For reportRenderStats declaration.
#include "SkPdfRenderer.h"

// Temp code to measure what operands fail.
template <typename T> class SkTDictWithDefaultConstructor : public SkTDict<T> {
public:
    SkTDictWithDefaultConstructor() : SkTDict<T>(10) {}
};

SkTDictWithDefaultConstructor<int> gRenderStats[kCount_SkPdfResult];

const char* gRenderStatsNames[kCount_SkPdfResult] = {
    "Success",
    "Partially implemented",
    "Not yet implemented",
    "Ignore Error",
    "Error",
    "Unsupported/Unknown"
};

// Declared in SkPdfRenderer.h. Should be moved to a central debugging location.
void reportPdfRenderStats() {
    for (int i = 0 ; i < kCount_SkPdfResult; i++) {
        SkTDict<int>::Iter iter(gRenderStats[i]);
        const char* key;
        int value = 0;
        while ((key = iter.next(&value)) != NULL) {
            printf("%s: %s -> count %i\n", gRenderStatsNames[i], key, value);
        }
    }
}

#include "SkPdfOps.h"

SkPdfResult PdfMainLooper::consumeToken(PdfToken& token) {
    if (token.fType == kKeyword_TokenType && token.fKeywordLength < 256)
    {
        PdfOperatorRenderer pdfOperatorRenderer = NULL;
        if (gPdfOps.find(token.fKeyword, token.fKeywordLength, &pdfOperatorRenderer) &&
                    pdfOperatorRenderer) {
            SkPdfTokenLooper* childLooper = NULL;
            // Main work is done by pdfOperatorRenderer(...)
            SkPdfResult result = pdfOperatorRenderer(fPdfContext, fCanvas, &childLooper);

            int cnt = 0;
            gRenderStats[result].find(token.fKeyword, token.fKeywordLength, &cnt);
            gRenderStats[result].set(token.fKeyword, token.fKeywordLength, cnt + 1);
            if (childLooper) {
                // FIXME (scroggo): Auto delete this.
                childLooper->setUp(this);
                childLooper->loop();
                delete childLooper;
            }
        } else {
            int cnt = 0;
            gRenderStats[kUnsupported_SkPdfResult].find(token.fKeyword,
                                                        token.fKeywordLength,
                                                        &cnt);
            gRenderStats[kUnsupported_SkPdfResult].set(token.fKeyword,
                                                       token.fKeywordLength,
                                                       cnt + 1);
        }
    }
    else if (token.fType == kObject_TokenType)
    {
        fPdfContext->fObjectStack.push( token.fObject );
    }
    else {
        // TODO(edisonn): store the keyword as a object, so we can track the location in file,
        //                and report where the error was triggered
        SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue, token.fKeyword, NULL,
                    fPdfContext);
        return kIgnoreError_SkPdfResult;
    }
    return kOK_SkPdfResult;
}

void PdfMainLooper::loop() {
    PdfToken token;
    // readToken defined in SkPdfTokenLooper.h
    // FIXME (scroggo): Remove readToken (which just calls fTokenizer->readToken, plus draws
    // some debugging info with PDF_DIFF_TRACE_IN_PNG)
    while (readToken(fTokenizer, &token)) {
        this->consumeToken(token);
    }
}
