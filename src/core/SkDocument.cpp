/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkDocument.h"
#include "include/core/SkStream.h"

SkDocument::SkDocument(SkWStream* stream) : fStream(stream), fState(kBetweenPages_State) {}

SkDocument::~SkDocument() {
    this->close();
}

static SkCanvas* trim(SkCanvas* canvas, SkScalar width, SkScalar height,
                      const SkRect* content) {
    if (content && canvas) {
        SkRect inner = *content;
        if (!inner.intersect({0, 0, width, height})) {
            return nullptr;
        }
        canvas->clipRect(inner);
        canvas->translate(inner.x(), inner.y());
    }
    return canvas;
}

SkCanvas* SkDocument::beginPage(SkScalar width, SkScalar height,
                                const SkRect* content) {
    if (width <= 0 || height <= 0 || kClosed_State == fState) {
        return nullptr;
    }
    if (kInPage_State == fState) {
        this->endPage();
    }
    SkASSERT(kBetweenPages_State == fState);
    fState = kInPage_State;
    return trim(this->onBeginPage(width, height), width, height, content);
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
    // we don't own the stream, but we mark it nullptr since we can
    // no longer write to it.
    fStream = nullptr;
}
