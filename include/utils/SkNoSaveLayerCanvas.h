/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNoSaveLayerCanvas_DEFINED
#define SkNoSaveLayerCanvas_DEFINED

#include "SkCanvas.h"
#include "SkRRect.h"

// The NoSaveLayerCanvas is used to play back SkPictures when the saveLayer
// functionality isn't required (e.g., during analysis of the draw calls).
// It also simplifies the clipping calls to only use rectangles.
class SK_API SkNoSaveLayerCanvas : public SkCanvas {
public:
    SkNoSaveLayerCanvas(SkBaseDevice* device)
        : INHERITED(device, kConservativeRasterClip_InitFlag)
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
