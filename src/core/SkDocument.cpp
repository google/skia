/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkDocument.h"
#include "SkStream.h"

SkDocument::SkDocument(SkWStream* stream, void (*doneProc)(SkWStream*, bool)) {
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
        return nullptr;
    }

    SkRect outer = SkRect::MakeWH(width, height);
    SkRect inner;
    if (content) {
        inner = *content;
        if (!inner.intersect(outer)) {
            return nullptr;
        }
    } else {
        inner = outer;
    }

    for (;;) {
        switch (fState) {
            case kBetweenPages_State: {
                fState = kInPage_State;
                SkCanvas* canvas = this->onBeginPage(width, height);
                if (content) {
                    canvas->clipRect(inner);
                    canvas->translate(inner.x(), inner.y());
                }
                return canvas;
            }
            case kInPage_State:
                this->endPage();
                break;
            case kClosed_State:
                return nullptr;
        }
    }
    SkDEBUGFAIL("never get here");
    return nullptr;
}

void SkDocument::endPage() {
    if (kInPage_State == fState) {
        fState = kBetweenPages_State;
        this->onEndPage();
    }
}

void SkDocument::close() {
    for (;;) {
        switch (fState) {
            case kBetweenPages_State: {
                fState = kClosed_State;
                this->onClose(fStream);

                if (fDoneProc) {
                    fDoneProc(fStream, false);
                }
                // we don't own the stream, but we mark it nullptr since we can
                // no longer write to it.
                fStream = nullptr;
                return;
            }
            case kInPage_State:
                this->endPage();
                break;
            case kClosed_State:
                return;
        }
    }
}

void SkDocument::abort() {
    this->onAbort();

    fState = kClosed_State;
    if (fDoneProc) {
        fDoneProc(fStream, true);
    }
    // we don't own the stream, but we mark it nullptr since we can
    // no longer write to it.
    fStream = nullptr;
}
