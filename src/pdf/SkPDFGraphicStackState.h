// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPDFGraphicStackState_DEFINED
#define SkPDFGraphicStackState_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkScalar.h"
#include "src/core/SkClipStack.h"

class SkDynamicMemoryWStream;

// It is important to not confuse SkPDFGraphicStackState with SkPDFGraphicState, the
// later being our representation of an object in the PDF file.
struct SkPDFGraphicStackState {
    struct Entry {
        SkMatrix fMatrix = SkMatrix::I();
        uint32_t fClipStackGenID = SkClipStack::kWideOpenGenID;
        SkColor4f fColor = {0, 0, 0, 1};
        SkScalar fTextScaleX = 1;  // Zero means we don't care what the value is.
        int fShaderIndex = -1;
        int fGraphicStateIndex = -1;
    };
    // Must use stack for matrix, and for clip, plus one for no matrix or clip.
    static constexpr int kMaxStackDepth = 2;
    Entry fEntries[kMaxStackDepth + 1];
    int fStackDepth = 0;
    SkDynamicMemoryWStream* fContentStream;

    SkPDFGraphicStackState(SkDynamicMemoryWStream* s = nullptr) : fContentStream(s) {}
    void updateClip(const SkClipStack* clipStack, const SkIRect& bounds);
    void updateMatrix(const SkMatrix& matrix);
    void updateDrawingState(const Entry& state);
    void push();
    void pop();
    void drainStack();
    Entry* currentEntry() { return &fEntries[fStackDepth]; }
};

#endif  // SkPDFGraphicStackState_DEFINED
