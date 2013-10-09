/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDocument.h"
#include "SkStream.h"

SK_DEFINE_INST_COUNT(SkDocument)

SkDocument::SkDocument(SkWStream* stream, void (*doneProc)(SkWStream*)) {
    fStream = stream;   // we do not own this object.
    fDoneProc = doneProc;
    fState = kBetweenPages_State;
}

SkDocument::~SkDocument() {
    this->close();
}

SkCanvas* SkDocument::beginPage(SkScalar width, SkScalar height,
                                const SkRect* content) {
    if (width <= 0 || height <= 0) {
        return NULL;
    }

    SkRect outer = SkRect::MakeWH(width, height);
    SkRect inner;
    if (content) {
        inner = *content;
        if (!inner.intersect(outer)) {
            return NULL;
        }
    } else {
        inner = outer;
    }

    for (;;) {
        switch (fState) {
            case kBetweenPages_State:
                fState = kInPage_State;
                return this->onBeginPage(width, height, inner);
            case kInPage_State:
                this->endPage();
                break;
            case kClosed_State:
                return NULL;
        }
    }
    SkDEBUGFAIL("never get here");
    return NULL;
}

void SkDocument::endPage() {
    if (kInPage_State == fState) {
        fState = kBetweenPages_State;
        this->onEndPage();
    }
}

bool SkDocument::close() {
    for (;;) {
        switch (fState) {
            case kBetweenPages_State: {
                fState = kClosed_State;
                bool success = this->onClose(fStream);

                if (fDoneProc) {
                    fDoneProc(fStream);
                }
                // we don't own the stream, but we mark it NULL since we can
                // no longer write to it.
                fStream = NULL;
                return success;
            }
            case kInPage_State:
                this->endPage();
                break;
            case kClosed_State:
                return false;
        }
    }
}

void SkDocument::abort() {
    fState = kClosed_State;
}
