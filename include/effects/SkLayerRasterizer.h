
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkLayerRasterizer_DEFINED
#define SkLayerRasterizer_DEFINED

#include "SkRasterizer.h"
#include "SkDeque.h"
#include "SkScalar.h"

class SkPaint;

class SK_API SkLayerRasterizer : public SkRasterizer {
public:
            SkLayerRasterizer();
    virtual ~SkLayerRasterizer();

    void addLayer(const SkPaint& paint) {
        this->addLayer(paint, 0, 0);
    }

    /**    Add a new layer (above any previous layers) to the rasterizer.
        The layer will extract those fields that affect the mask from
        the specified paint, but will not retain a reference to the paint
        object itself, so it may be reused without danger of side-effects.
    */
    void addLayer(const SkPaint& paint, SkScalar dx, SkScalar dy);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLayerRasterizer)

protected:
    SkLayerRasterizer(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    // override from SkRasterizer
    virtual bool onRasterize(const SkPath& path, const SkMatrix& matrix,
                             const SkIRect* clipBounds,
                             SkMask* mask, SkMask::CreateMode mode) const;

private:
    SkDeque fLayers;

    typedef SkRasterizer INHERITED;
};

#endif
