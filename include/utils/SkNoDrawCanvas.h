/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNoDrawCanvas_DEFINED
#define SkNoDrawCanvas_DEFINED

#include "SkCanvas.h"
#include "SkVertices.h"

struct SkIRect;

// SkNoDrawCanvas is a helper for SkCanvas subclasses which do not need to
// actually rasterize (e.g., analysis of the draw calls).
//
// It provides the following simplifications:
//
//   * not backed by any device/pixels
//   * conservative clipping (clipping calls only use rectangles)
//
class SK_API SkNoDrawCanvas : public SkCanvas {
public:
    SkNoDrawCanvas(int width, int height);

    // TODO: investigate the users of this ctor.
    SkNoDrawCanvas(const SkIRect&);

    // Optimization to reset state to be the same as after construction.
    void resetCanvas(int width, int height) {
        resetForNextPicture(SkIRect::MakeWH(width, height));
    }

protected:
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override;

    // Stub out all onDraw virtuals to do nothing
#define X(func, ...) void func(__VA_ARGS__) override {}
#include "SkCanvasDrawVirtuals.inc"

private:
    typedef SkCanvas INHERITED;
};

#endif // SkNoDrawCanvas_DEFINED
