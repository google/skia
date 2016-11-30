/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNoDrawCanvas_DEFINED
#define SkNoDrawCanvas_DEFINED

#include "SkCanvas.h"
#include "SkRect.h"

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
    SkNoDrawCanvas(int width, int height)
        : INHERITED(SkIRect::MakeWH(width, height), kConservativeRasterClip_InitFlag)
    {}

protected:
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
        (void)this->INHERITED::getSaveLayerStrategy(rec);
        return kNoLayer_SaveLayerStrategy;
    }

private:
    typedef SkCanvas INHERITED;
};

#endif // SkNoSaveLayerCanvas_DEFINED
